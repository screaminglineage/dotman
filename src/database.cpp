#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "database.h"
#include "utils.h"
#include "sqlite_orm/sqlite_orm.h"

bool configExists(Storage& storage, std::string_view programTitle,
                         std::string_view programTag) {
    using namespace sqlite_orm;
    auto data = storage.select(&ProgramData::id,
                               where(c(&ProgramData::title) == programTitle and
                                     c(&ProgramData::tag) == programTag));

    return !data.empty();
}

bool insertConfig(Storage& storage, ProgramData& cfg, std::vector<ConfigFile>& cfgFiles) {
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
        return true;  //  commit db
    });
    return true;
}

int getProgramId(Storage& storage, std::string_view programTitle, std::string_view programTag) {
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

std::vector<ConfigFile> getProgramData(Storage& storage, int programId) {
    using namespace sqlite_orm;
    auto fileData = storage.select(
        object<ConfigFile>(), where(c(&ConfigFile::programId) == programId));

    return fileData;
}

// TODO: return a vector of all the paths that need to be synced
// when file in db but not in backup, file in db but older than actual, file not in db
void syncFiles(Storage& storage, int programId, bool checkBackupDir) {
    using namespace sqlite_orm;
    namespace fs = std::filesystem;

    auto program = storage.get<ProgramData>(programId);

    ASSERT_WITH_MSG(
        fs::exists(program.configDir),
        std::format("Config directory: {} doesnt exist!", program.configDir));

    for (const auto& file : fs::recursive_directory_iterator(program.configDir)) {

        auto dbFiles = storage.get_all<ConfigFile>(
            where(c(&ConfigFile::filePath) == file.path().string()));

        ASSERT_WITH_MSG(dbFiles.size() <= 1,
                        "Cannot have multiple files with same path");

        if (dbFiles.empty()) {
            std::cout << "File not in Database!" << std::endl;
            // add file to db
            // push file into the to-be-copied vector
            continue;
        }
        auto lastWriteTimeFile = getlastWriteTime(file);

        // checks if the file is not present in the backup dir
        if (checkBackupDir) {
            // replaces prefix `configPath` with `backupPath` in the full file path
            fs::path originalConfigPath = file.path().lexically_relative(paths::configPath).lexically_normal();
            fs::path backupConfigPath = paths::backupPath / originalConfigPath;

            if (!fs::exists(backupConfigPath)) {
                std::cout << "unsynced file: " << backupConfigPath << '\n';
                // push file into the to-be-copied vector
                continue;
            }
        }
        std::cout << ((lastWriteTimeFile > dbFiles[0].lastModified)? "Filesystem is newer!": "Database is synced!") << '\n';
        // update lastWriteTime in db
        // push file into the to-be-copied vector
    } 
}
