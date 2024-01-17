#include "database.h"
#include <chrono>
#include <filesystem>
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

// struct File {
//     fs::path filepath{};
//     time_t lastModified{};
// };
//
// struct ConfigFilesData {
//     std::string programTitle{};
//     fs::path directory{};
//     std::vector<File> files{};
//
//     void print() {
//         std::cout << '\n';
//         std::cout << "Title: " << this->programTitle << '\n';
//         std::cout << "Path: " << this->directory << '\n';
//
//         std::cout << "Files:\n";
//         auto i = 1;
//         for (const auto& file : this->files) {
//             std::cout << '[' << i << "] " << file.filepath << " ("
//                       << file.lastModified << ")\n";
//             i++;
//         }
//     }
// };

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

void createDb();

int main(int argc, char* argv[]) {

    std::string_view program{*argv};
    VecStr args(argv + 1, argv + argc);

    auto argOptions = parseArguments(args);

    std::cerr << program << '\n' << "Args: ";
    for (const auto& it : argOptions) {
        std::cerr << it.first << ' ';
        for (const auto& j : it.second) {
            std::cerr << j << ' ';
        }
    }
    std::cerr << '\n';

    auto programTitles{argOptions["add"]};

    if (programTitles.empty()) {
        std::cerr << "error: no path specified!\n";
        return 1;
    }

    for (const auto& programTitle : programTitles) {
        auto dirPath{configPath / fs::path{programTitle}};

        ProgramData cfg{.programName = std::string{programTitle},
                        .configPath = dirPath};
        if (!fs::exists(dirPath)) {
            std::cerr << "error: " << dirPath << " not found!\n";
            return 1;
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
        createDb(cfg, files);
    }

    return 0;
}
