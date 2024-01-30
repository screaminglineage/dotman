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
const std::filesystem::path dbFilePath{"./db.sqlite"};
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

inline auto initDb() {
    std::cout << "Initialising DB: " << paths::dbFilePath << "...\n";

    using namespace sqlite_orm;
    auto storage = make_storage(
        paths::dbFilePath,
        make_table(
            "programs",
            make_column("id", &ProgramData::id, primary_key().autoincrement()),
            make_column("tag", &ProgramData::tag),
            make_column("title", &ProgramData::title),
            make_column("config_path", &ProgramData::configDir)));
        
    storage.sync_schema();

    std::cout << "DB Initialised!\n";

    return storage;
}

using Storage = decltype(initDb());

// check if a program with the given title and tag already exists
bool configExists(Storage& storage, std::string_view programTitle, std::string_view programTag);
    
// inserts configs for a program  into the DB
bool insertNewConfig(Storage& storage, ProgramData& cfg);

// Returns the ID for a program given its title and tag
int getProgramId(Storage& storage, std::string_view programTitle, std::string_view programTag);

// Returns a vector of all the paths in a pair (from, to) 
// for paths that need to be synced. Also checks if a file
// doesnt exist in the backup dir when checkBackupDir is true
// std::vector<std::pair<std::filesystem::path, std::filesystem::path>> 
//     syncFiles(Storage& storage, int programId, bool checkBackupDir = true);

