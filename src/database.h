#pragma once
#include "sqlite_orm/sqlite_orm.h"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

// TODO: remove these eventually
namespace paths {
const std::filesystem::path configPath{"/home/aditya/.config/"};
const std::filesystem::path backupPath{"/mnt/Other-Stuff/Programming/C++/dotman/backup_temp/"};
}

#define ASSERT_WITH_MSG(cond, msg)                                             \
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

struct ConfigProgram {
    ProgramData data;
    std::vector<ConfigFile> files;
};

inline auto createDb() {
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

using Storage = decltype(createDb());

// check if a program with the given title and tag already exists
inline bool configExists(Storage& storage, std::string_view programTitle,
                         std::string_view programTag) {
    using namespace sqlite_orm;
    auto data = storage.select(&ProgramData::id,
                               where(c(&ProgramData::title) == programTitle and
                                     c(&ProgramData::tag) == programTag));

    return !data.empty();
}

// inserts configs for a program and all of its files into the DB
inline bool insertConfig(Storage& storage, ProgramData& cfg,
                        std::vector<ConfigFile>& cfgFiles) {

    if (configExists(storage, cfg.title, cfg.tag)) {
        return false;
    }

    int programId = storage.insert(cfg);
    std::cout << "insertedId = " << programId << '\n';
    cfg.id = programId;

    storage.transaction([&] {
        for(auto& cfgFile: cfgFiles) {
            cfgFile.programId = programId;
            int fileId = storage.insert(cfgFile);
            cfgFile.id = fileId;
        }
        return true;  //  commit
    });
    return true;
}

// Returns the ID for a program given its title and tag
inline int getProgramId(Storage& storage, std::string_view programTitle,
                        std::string_view programTag = "primary") {
    using namespace sqlite_orm;
    auto data = storage.select(&ProgramData::id,
                               where(c(&ProgramData::title) == programTitle and
                                     c(&ProgramData::tag) == programTag));

    ASSERT_WITH_MSG(
        data.size() == 1,
        std::format(
            "Title: `{}` and Tag: `{}` must uniquely identify a single element",
            programTitle, programTag));

    return data[0];
}

// Get the all the file data for a particular program
// The return value is an iterable object with each element of type
// ConfigFile
inline auto getProgramData(Storage& storage, int programId) {
    using namespace sqlite_orm;
    auto fileData = storage.select(
        object<ConfigFile>(), where(c(&ConfigFile::programId) == programId));

    return fileData;
}


// TODO: return a vector of all the paths that need to be synced
// when file in db but not in backup, file in db but older than actual, file not in db
//
// if checkBackupDir is set to true, the function checks for each file in 
// the backup directory and if the file doesnt exist then copies it over
inline void syncFiles(Storage& storage, int programId, bool checkBackupDir = true) {
    using namespace sqlite_orm;
    namespace fs = std::filesystem;

    auto program = storage.get<ProgramData>(programId);

    ASSERT_WITH_MSG(
        fs::exists(program.configDir),
        std::format("Config directory: {} doesnt exist!", program.configDir));

    for (const auto& file :
         fs::recursive_directory_iterator(program.configDir)) {

        auto dbFiles = storage.get_all<ConfigFile>(
            where(c(&ConfigFile::filePath) == file.path().string()));

        ASSERT_WITH_MSG(dbFiles.size() <= 1,
                        "Cannot have multiple files with same path");

        if (!dbFiles.empty()) {
            // TODO: extract out getting the time into a function
            namespace chrono = std::chrono;

            auto time = fs::last_write_time(file);
            auto lastWriteTimeFile = chrono::system_clock::to_time_t(
                chrono::clock_cast<chrono::system_clock>(time));

            if (checkBackupDir) {
                // replaces prefix `configPath` with `backupPath` in the full file path
                fs::path originalConfigPath = file.path().lexically_relative(paths::configPath).lexically_normal();
                fs::path backupConfigPath = paths::backupPath / originalConfigPath;

                if (!fs::exists(backupConfigPath)) {
                    std::cout << "unsynced file: " << backupConfigPath << '\n';
                }
            }

            std::cout << ((lastWriteTimeFile > dbFiles[0].lastModified)? "Filesystem is newer!": "Database is synced!") << '\n';

        } else {
            std::cout << "File not in Database!" << std::endl;
        }
    }
}
