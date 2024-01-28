#pragma once
#include "sqlite_orm/sqlite_orm.h"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

// TODO: remove these eventually
namespace paths {
const std::filesystem::path configPath{"/home/aditya/.config/"};
const std::filesystem::path backupPath{"/mnt/Other-Stuff/Programming/C++/dotman/backup_temp/"};
}

#define dotman_assert(cond, msg)                                           \
do {                                                                       \
    if (!(cond)) {                                                         \
        std::ostringstream str;                                            \
        str << "Assertion Failed! " << msg;                                \
        std::cerr << str.str();                                            \
        std::abort();                                                      \
    }                                                                      \
} while (0)

struct ProgramData {
    // int uuid;
    int id{-1};
    std::string tag{"primary"};
    std::string title{};
    std::string configDir{};
};

struct ConfigFile {
    // int uuid;
    int id{-1};
    int programId{-1};
    std::string filePath{};
    std::time_t lastModified{};
};

inline auto initDb() {
    std::cout << "Creating table...\n";

    using namespace sqlite_orm;
    auto storage = make_storage(
        "db.sqlite",
        make_table(
            "programs",
            make_column("id", &ProgramData::id, primary_key().autoincrement()),
            make_column("tag", &ProgramData::tag),
            make_column("title", &ProgramData::title),
            make_column("config_path", &ProgramData::configDir)),
        make_table(
            "config_files",
            make_column("id", &ConfigFile::id, primary_key().autoincrement()),
            make_column("program_id", &ConfigFile::programId),
            make_column("file_path", &ConfigFile::filePath),
            make_column("last_modified", &ConfigFile::lastModified),
            foreign_key(&ConfigFile::programId).references(&ProgramData::id)));

    storage.sync_schema();

    std::cout << "Created db.sqlite!\n";

    return storage;
}

using Storage = decltype(initDb());

// check if a program with the given title and tag already exists
bool configExists(Storage& storage, std::string_view programTitle, std::string_view programTag);
    
// inserts configs for a program and all of its files into the DB
bool insertNewConfig(Storage& storage, ProgramData& cfg, std::vector<ConfigFile>& cfgFiles);

// Returns the ID for a program given its title and tag
int getProgramId(Storage& storage, std::string_view programTitle, std::string_view programTag);

// Get the all the config file  data for a particular program
std::vector<ConfigFile> getConfigFiles(Storage& storage, int programId);


// TODO: return a vector of all the paths that need to be synced
// when file in db but not in backup, file in db but older than actual, file not in db
//
// if checkBackupDir is set to true, the function checks for each file in 
// the backup directory and if the file doesnt exist then copies it over
std::vector<std::pair<std::filesystem::path, std::filesystem::path>> 
    syncFiles(Storage& storage, int programId, bool checkBackupDir = true);

