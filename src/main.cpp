#include <chrono> // required for printing lastModified time even if the LSP doesnt detect that
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

struct File {
    fs::path filepath{};
    fs::file_time_type lastModified{};
};

struct ConfigFiles {
    std::string programTitle{};
    fs::path directory{};
    std::vector<File> files{};

    void print() {
        std::cout << '\n';
        std::cout << "Title: " << this->programTitle << '\n';
        std::cout << "Path: " << this->directory << '\n';

        std::cout << "Files:\n";
        auto i = 1;
        for (const auto& file : this->files) {
            std::cout << '[' << i << "] " << file.filepath << " ("
                      << file.lastModified << ")\n";
            i++;
        }
    }
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

        if (!fs::exists(dirPath)) {
            std::cerr << "error: " << dirPath << " not found!\n";
            return 1;
        }

        std::vector<File> files{};
        for (const auto& file : fs::recursive_directory_iterator(dirPath)) {
            File newFile{file, fs::last_write_time(file)};
            files.push_back(newFile);
        }

        ConfigFiles config{std::string{programTitle}, dirPath, files};
        config.print();
    }
    createDb();
    return 0;
}