#pragma once
#include "../include/sqlite_orm/sqlite_orm.h"
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

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
    std::string title;
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

inline auto createDb() {
    std::cout << "Creating table...\n";

    using namespace sqlite_orm;
    auto storage = make_storage(
        "db.sqlite",
        make_table(
            "config_programs",
            make_column("id", &ProgramData::id, primary_key().autoincrement()),
            make_column("tag", &ProgramData::tag),
            make_column("programName", &ProgramData::title),
            make_column("configPath", &ProgramData::configPath)),
        make_table(
            "config_files",
            make_column("id", &ConfigFile::id, primary_key().autoincrement()),
            make_column("program_id", &ConfigFile::programId),
            make_column("files", &ConfigFile::filePath),
            make_column("last_modified", &ConfigFile::lastModified),
            foreign_key(&ConfigFile::programId).references(&ProgramData::id)));

    storage.sync_schema();

    std::cout << "Created db.sqlite!\n";

    return storage;
}

inline void insertConfig(auto& storage, ProgramData& cfg,
                         std::vector<ConfigFile>& cfgFiles) {
    int programId = storage.insert(cfg);
    std::cout << "insertedId = " << programId << '\n';
    cfg.id = programId;

    for (auto& cfgFile : cfgFiles) {
        cfgFile.programId = programId;
        int fileId = storage.insert(cfgFile);
        std::cout << "insertedId = " << fileId << '\n';
        cfgFile.id = fileId;
    }
}

// Returns the data for the program given its title and tag
// The return value is an iterable object with each element of type ConfigFile
inline auto getProgramData(auto storage, std::string_view programTitle,
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

    int programId = data[0];
    std::cout << programId << '\n';

    auto fileData = storage.select(
        object<ConfigFile>(), where(c(&ConfigFile::programId) == programId));

    return fileData;
}
