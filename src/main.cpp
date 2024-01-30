#include "database.h"
#include "utils.h"
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace fs = std::filesystem;
using VecStr = std::vector<std::string_view>;

const std::unordered_set<std::string_view> options{"add", "sync"};

auto parseArguments(VecStr& args) {
    std::unordered_map<std::string_view, VecStr> argOptions{};

    for (auto it{args.begin()}; it < args.end(); ++it) {
        if (!options.contains(*it))
            continue;

        auto opt = *it;
        ++it;
        VecStr addArgs{};
        while (it < args.end() && !options.contains(*it)) {
            addArgs.push_back(*it);
            ++it;
        }
        argOptions.insert({opt, addArgs});
    }
    return argOptions;
}

// TODO: add option to also pass in the tags along with the program titles
bool addPrograms(Storage& storage, VecStr& programTitles) {
    for (const auto& programTitle : programTitles) {
        auto dirPath{paths::configPath / fs::path{programTitle}};

        ProgramData cfg{.title = std::string{programTitle},
                        .configDir = dirPath};
        if (!fs::exists(dirPath)) {
            std::cerr << "error: " << dirPath << " not found!\n";
            continue;
        }

        // add configs to DB
        if (!insertNewConfig(storage, cfg)) {
            std::cerr << std::format(
                "Config: {} with tag `{}` already exists!\n", cfg.title,
                cfg.tag);
            continue;
        }

        // copy over files into backup dir
        std::cout << std::format("Copying Files (`{}`) ...\n", programTitle);
        auto programBackupPath = paths::backupPath / fs::path{cfg.tag};
        if (!fs::exists(programBackupPath)) {
            fs::create_directory(programBackupPath);
        }
        fs::copy(dirPath, programBackupPath / fs::path{cfg.title}, 
                 fs::copy_options::recursive | fs::copy_options::update_existing);
    }
    return true;
}

int main(int argc, char* argv[]) {
    std::string_view program = *argv;
    VecStr args(argv + 1, argv + argc);

    auto argOptions = parseArguments(args);
    auto programTitles = argOptions["add"];
    auto syncPrograms = argOptions["sync"];

    if (programTitles.empty() && syncPrograms.empty()) {
        std::cerr << std::format("{}: no valid options specified!\n", program);
        return 1;
    }

    auto storage = initDb();
    if (!addPrograms(storage, programTitles))
        return 1;

    for (const auto& prog: syncPrograms) {
        if (!configExists(storage, prog, "primary")) {
            std::cerr << std::format("{}: no such program added: `{}`\n", program, prog);
            return 1;
        }
        if (!syncFiles(storage, getProgramId(storage, prog, "primary"))) {
            std::cerr << std::format("{}: backup directory not found for `{}`\n", program, prog);
            return 1;
        }
    }

    return 0;
}
