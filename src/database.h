#pragma once
#include <chrono>
#include <string>

struct ConfigProgram {
    // int uuid;
    int id{-1};
    std::string tag{"primary"};
    std::string programName;
    std::string configPath;
};

struct ConfigFile {
    // int uuid;
    int id{-1};
    int programId{-1};
    std::string filePath;
    std::time_t lastModified;
};

void createDb(ConfigProgram cfg, std::vector<ConfigFile> cfgFiles);
