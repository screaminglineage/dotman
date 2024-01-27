#include "database.h"
#include "utils.h"
#include <chrono>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace fs = std::filesystem;
using namespace std::chrono_literals;

const std::unordered_set<std::string_view> options{"add", "sync"};

using VecStr = std::vector<std::string_view>;

struct CLIOption {
    const char* long_option{};
    const char* short_option{};
};

auto parseArguments(VecStr& args) {
    std::unordered_map<std::string_view, VecStr> argOptions{};

    for (auto it{args.begin()}; it < args.end(); ++it) {
        for (const auto& opt : options) {
            if (*it != opt)
                continue;

            ++it;
            VecStr addArgs{};
            while (it < args.end() && !options.contains(*it)) {
                addArgs.push_back(*it);
                ++it;
            }
            argOptions.insert({opt, addArgs});
        }
    }
    return argOptions;
}

bool addPrograms(auto& storage, VecStr& programTitles) {
    for (const auto& programTitle : programTitles) {
        auto dirPath{paths::configPath / fs::path{programTitle}};

        ProgramData cfg{.title = std::string{programTitle},
                        .configDir = dirPath};
        if (!fs::exists(dirPath)) {
            std::cerr << "error: " << dirPath << " not found!\n";
            continue;
        }

        std::vector<ConfigFile> files{};
        for (const auto& file : fs::recursive_directory_iterator(dirPath)) {
            auto lastWriteTime = getlastWriteTime(file);
            files.push_back(ConfigFile{.filePath = file.path().string(),
                                       .lastModified = lastWriteTime});
        }

        // add configs to DB
        if (!insertConfig(storage, cfg, files)) {
            std::cerr << std::format(
                "Config: {} with tag `{}` already exists!\n", cfg.title,
                cfg.tag);
            continue;
        }

        // copy over files into backup dir
        std::cout << std::format("Copying Files (`{}`) ...\n", programTitle);
        auto programBackupPath{paths::backupPath / fs::path{programTitle}};
        fs::copy(dirPath, programBackupPath, fs::copy_options::recursive | fs::copy_options::update_existing);
    }
    return true;
}

int main(int argc, char* argv[]) {

    std::string_view program{*argv};
    VecStr args(argv + 1, argv + argc);

    auto argOptions = parseArguments(args);
    auto programTitles = argOptions["add"];
    auto syncPrograms = argOptions["sync"];

    if (programTitles.empty() && syncPrograms.empty()) {
        std::cerr << "Error: no valid options specified!\n";
        return 1;
    }

    auto storage = createDb();
    if (!addPrograms(storage, programTitles))
        return 1;

    // for testing
    auto data1 = getProgramData(storage, getProgramId(storage, "kitty", "primary"));
    for (const auto& file : data1) {
        std::cout << file.filePath << ' ' << file.lastModified << std::endl;
    }

    if (!syncPrograms.empty() && !syncPrograms[0].empty()) {
        if (!configExists(storage, syncPrograms[0], "primary")) {
            std::cerr << std::format("Error: no such program added: `{}`\n", syncPrograms[0]);
            return 1;
        }
        syncFiles(storage, getProgramId(storage, syncPrograms[0], "primary"));
    }

    return 0;
}
