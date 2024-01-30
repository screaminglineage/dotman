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

    DOTMAN_ASSERT(
        data.size() == 1,
        std::format(
            "Title: `{}` and Tag: `{}` must uniquely identify a single element",
            programTitle, programTag));

    return data[0];
}

bool syncFiles(Storage& storage, int programId) {
    using namespace sqlite_orm;
    auto program = storage.get<ProgramData>(programId);

    DOTMAN_ASSERT(
        fs::exists(program.configDir),
        std::format("Config directory: {} doesnt exist!", program.configDir));

    auto backupDir = paths::backupPath / fs::path{program.tag} / fs::path{program.title};
    if (!fs::exists(backupDir)) {
        return false;
    }

    for (const auto& file : fs::recursive_directory_iterator(backupDir)) {
        // replaces the backup path prefix with its original path
        // (/a/b/file.txt -> /c/d/file.txt)
        auto fileRelativePath = file.path()
            .lexically_relative(backupDir)
            .lexically_normal();
        auto originalPath = program.configDir / fileRelativePath;

        std::cout << originalPath.string() << '\n';

        if (!fs::exists(originalPath)) {
            fs::remove(file);
        }
    }
    fs::copy(program.configDir, backupDir,
             fs::copy_options::recursive | fs::copy_options::update_existing);
    return true;
}
