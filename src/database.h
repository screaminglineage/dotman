#pragma once
#include "sqlite_orm/sqlite_orm.h"
#include <chrono>
#include <filesystem>
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
            make_column("configPath", &ProgramData::configDir)),
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

// check if a program with the given title and tag already exists
inline bool configExists(auto& storage, std::string_view programTitle,
                         std::string_view programTag) {
    using namespace sqlite_orm;
    auto data = storage.select(&ProgramData::id,
                               where(c(&ProgramData::title) == programTitle and
                                     c(&ProgramData::tag) == programTag));

    return !data.empty();
}

// inserts configs for a program and all of its files into the DB
inline int insertConfig(auto& storage, ProgramData& cfg,
                        std::vector<ConfigFile>& cfgFiles) {

    if (configExists(storage, cfg.title, cfg.tag)) {
        return 1;
    }

    int programId = storage.insert(cfg);
    std::cout << "insertedId = " << programId << '\n';
    cfg.id = programId;

    for (auto& cfgFile : cfgFiles) {
        cfgFile.programId = programId;
        int fileId = storage.insert(cfgFile);
        cfgFile.id = fileId;
    }
    return 0;
}

// Returns the ID for a program given its title and tag
inline int getProgramId(auto storage, std::string_view programTitle,
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
inline auto getProgramData(auto storage, int programId) {
    using namespace sqlite_orm;
    auto fileData = storage.select(
        object<ConfigFile>(), where(c(&ConfigFile::programId) == programId));

    return fileData;
}

inline void syncFiles(auto storage, int programId) {
    using namespace sqlite_orm;
    namespace fs = std::filesystem;

    auto configDir = storage.select(&ProgramData::configDir,
                                    where(c(&ProgramData::id) == programId));
    ASSERT_WITH_MSG(
        fs::exists(configDir),
        std::format("Config directory: {} doesnt exist!", configDir));

    // loop through each file in config dir
    // check if it exists in db
    // if it does not then add it to be copied and then added to db
    // if it exists but has a newer time then do the same
    // otherwise continue

    // for (const auto& file : fs::recursive_directory_iterator(configDir)) {
    //     storage.select()
    // }
}
