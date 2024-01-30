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

#define DOTMAN_ASSERT(cond, msg)                                           \
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

// Check if a program with the given title and tag already exists
bool configExists(Storage& storage, std::string_view programTitle, std::string_view programTag);
    
// Insert configs for a program  into the DB
bool insertNewConfig(Storage& storage, ProgramData& cfg);

// Return the ID for a program given its title and tag
int getProgramId(Storage& storage, std::string_view programTitle, std::string_view programTag);

// Sync between program config path and backup directory
// deleting any files which exist in backup but not in config
// and updating others
// Returns false if backup directory doesnt exist
bool syncFiles(Storage& storage, int programId);
