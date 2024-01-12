#include <array>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

const std::filesystem::path config_path{"/home/aditya/.config/"};
const std::array<const char*, 1> options{"add"};

using VecStr = std::vector<std::string_view>;

struct CLIOption {
    const char* long_option{};
    const char* short_option{};
};

struct ConfigFiles {
    std::string programTitle;
    fs::path directory;
    std::vector<fs::path> files;

    void print() {
        std::cout << '\n';
        std::cout << "Title: " << this->programTitle << '\n';
        std::cout << "Path: " << this->directory << '\n';

        std::cout << "Files:\n";
        auto i = 1;
        for (const auto& file : this->files) {
            std::cout << '[' << i << "] " << file << '\n';
            i++;
        }
    }
};

auto parseArguments(VecStr& args) {
    std::unordered_map<std::string_view, std::string_view> argOptions{};

    for (auto it{args.begin()}; it != args.end(); ++it) {
        for (const auto& opt : options) {
            if (*it == opt && (it + 1) < args.end()) {
                argOptions.insert({opt, *(++it)});
            }
        }
    }
    return argOptions;
}

int main(int argc, char* argv[]) {
    std::string_view program{*argv};
    VecStr args(argv + 1, argv + argc);

    auto argOptions = parseArguments(args);

    std::cout << program << '\n' << "Args: ";
    for (const auto& it : argOptions) {
        std::cout << it.first << ' ' << it.second << ' ';
    }
    std::cout << '\n';

    auto programTitle = argOptions["add"];
    if (programTitle.empty()) {
        std::cerr << "error: no path specified!\n";
        return 1;
    }

    auto dirPath{config_path / fs::path{programTitle}};

    if (!fs::exists(dirPath)) {
        std::cerr << "error: " << dirPath << " not found!\n";
        return 1;
    }

    std::vector<fs::path> files{};
    for (const auto& file : fs::recursive_directory_iterator(dirPath)) {
        files.push_back(file);
    }

    ConfigFiles config{std::string{programTitle}, dirPath, files};
    config.print();

    return 0;
}
