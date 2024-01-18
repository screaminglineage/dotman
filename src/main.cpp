#include "database.h"
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

const std::filesystem::path configPath{"/home/aditya/.config/"};
const std::unordered_set<std::string_view> options{"add"};

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
                break;

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

int addPrograms(auto& storage, VecStr& programTitles) {
    for (const auto& programTitle : programTitles) {
        auto dirPath{configPath / fs::path{programTitle}};

        ProgramData cfg{.title = std::string{programTitle},
                        .configDir = dirPath};
        if (!fs::exists(dirPath)) {
            std::cerr << "error: " << dirPath << " not found!\n";
            continue;
        }

        std::vector<ConfigFile> files{};
        for (const auto& file : fs::recursive_directory_iterator(dirPath)) {
            namespace chrono = std::chrono;
            auto time = fs::last_write_time(file);
            auto lastWriteTime = chrono::system_clock::to_time_t(
                chrono::clock_cast<chrono::system_clock>(time));

            files.push_back(ConfigFile{.filePath = file.path().string(),
                                       .lastModified = lastWriteTime});
        }

        // add configs to DB
        if (insertConfig(storage, cfg, files) != 0) {
            std::cerr << std::format(
                "Config: {} with tag `{}` already exists!\n", cfg.title,
                cfg.tag);
            continue;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {

    std::string_view program{*argv};
    VecStr args(argv + 1, argv + argc);

    auto argOptions = parseArguments(args);
    auto programTitles{argOptions["add"]};

    if (programTitles.empty()) {
        std::cerr << "error: no path specified!\n";
        return 1;
    }

    auto storage = createDb();
    if (addPrograms(storage, programTitles) != 0)
        return 1;

    auto data1 = getProgramData(storage, getProgramId(storage, "kitty"));
    for (const auto& file : data1) {
        std::cout << file.filePath << ' ' << file.lastModified << std::endl;
    }

    auto data2 = getProgramData(storage, getProgramId(storage, "kitty"));
    for (const auto& file : data2) {
        std::cout << file.filePath << ' ' << file.lastModified << std::endl;
    }

    return 0;
}
