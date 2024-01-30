#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "database.h"
#include "sqlite_orm/sqlite_orm.h"
#include "utils.h"

namespace fs = std::filesystem;

bool configExists(Storage& storage, std::string_view programTitle,
                  std::string_view programTag) {
    using namespace sqlite_orm;
    auto data = storage.select(&ProgramData::id,
                               where(c(&ProgramData::title) == programTitle and
                                     c(&ProgramData::tag) == programTag));

    return !data.empty();
}

bool insertNewConfig(Storage& storage, ProgramData& cfg) {
    if (configExists(storage, cfg.title, cfg.tag)) {
        return false;
    }

    auto programId = storage.insert(cfg);
    std::cout << "insertedId = " << programId << '\n';
    cfg.id = programId;
    return true;
}

int getProgramId(Storage& storage, std::string_view programTitle,
                 std::string_view programTag) {
    using namespace sqlite_orm;
    auto data = storage.select(&ProgramData::id,
                               where(c(&ProgramData::title) == programTitle and
                                     c(&ProgramData::tag) == programTag));

    dotman_assert(
        data.size() == 1,
        std::format(
            "Title: `{}` and Tag: `{}` must uniquely identify a single element",
            programTitle, programTag));

    return data[0];
}

// Returns a vector of all the paths in a pair (from, to)
// for paths that need to be synced. Also checks if a file
// doesnt exist in the backup dir when checkBackupDir is true
// std::vector<std::pair<fs::path, fs::path>>
// syncFiles(Storage& storage, int programId, bool checkBackupDir) {
//     using namespace sqlite_orm;
//
//     auto program = storage.get<ProgramData>(programId);
//
//     dotman_assert(
//         fs::exists(program.configDir),
//         std::format("Config directory: {} doesnt exist!", program.configDir));
//
//     std::vector<std::pair<fs::path, fs::path>> filesToCopy{};
//     // putting all db accesses in 1 transaction to prevent repeated opening and
//     // closing
//     storage.begin_transaction();
//     for (const auto& file :
//          fs::recursive_directory_iterator(program.configDir)) {
//
//         auto dbFiles = storage.get_all<ConfigFile>(
//             where(c(&ConfigFile::filePath) == file.path().string()));
//
//         dotman_assert(dbFiles.size() <= 1,
//                       "Cannot have multiple files with same path");
//
//         // replaces the original path prefix with a new one in the backup
//         // directory /home/user/.config/program/file.txt ->
//         // /tmp/backup/program/file1.txt
//         auto programRelativePath = file.path()
//                                        .lexically_relative(paths::configPath)
//                                        .lexically_normal();
//         auto backupPath =
//             paths::backupPath / fs::path{program.tag} / programRelativePath;
//         auto lastWriteTimeFile = getlastWriteTime(file);
//
//         // add file to db if missing
//         if (dbFiles.empty()) {
//             std::cerr << "File not in DB: " << file.path().string() << '\n';
//             auto cfgFile = ConfigFile{.id = -1,
//                                       .programId = programId,
//                                       .filePath = file.path().string(),
//                                       .lastModified = lastWriteTimeFile};
//             auto fileId = storage.insert(cfgFile);
//             cfgFile.id = fileId;
//
//             filesToCopy.emplace_back(file.path(), backupPath);
//             continue;
//         }
//
//         // checks if the file is not present in the backup dir
//         if (checkBackupDir && !fs::exists(backupPath)) {
//             std::cerr << "Unsynced file: " << backupPath << '\n';
//             filesToCopy.emplace_back(file.path(), backupPath);
//             continue;
//         }
//
//         auto dbFile = dbFiles[0];
//         // update lastWriteTime in db if older
//         if (lastWriteTimeFile > dbFile.lastModified) {
//             std::cerr << "File newer than backup: " << file.path().string()
//                       << '\n';
//             dbFile.lastModified = lastWriteTimeFile;
//             storage.update(dbFile);
//             filesToCopy.emplace_back(file.path(), backupPath);
//         }
//     }
//     storage.commit();
//     return filesToCopy;
// }
