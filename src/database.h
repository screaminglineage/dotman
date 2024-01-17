#pragma once
#include <chrono>
#include <string>

struct Config {
    // int uuid;
    int id{-1};
    std::string tag{"primary"};
    std::string programName;
    std::string configPath;
};

struct ConfigFiles {
    // int uuid;
    int id{-1};
    int programId{-1};
    std::string filePath;
    std::time_t lastModified;
};

void createDb(Config cfg, std::vector<ConfigFiles> cfgFiles);
