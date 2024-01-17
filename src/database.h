#pragma once
#include <chrono>
#include <string>
#include <vector>

struct ProgramData {
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

struct ConfigProgram {
    ProgramData data;
    std::vector<ConfigFile> files;
};

void createDb(ProgramData& cfg, std::vector<ConfigFile>& cfgFiles);
