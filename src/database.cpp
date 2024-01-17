#include "../include/sqlite_orm/sqlite_orm.h"
#include <iostream>
#include <string>
#include <vector>

#include "database.h"

void insertConfig(auto storage, ConfigProgram cfg,
                  std::vector<ConfigFile> cfgFiles) {
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

void createDb(ConfigProgram cfg, std::vector<ConfigFile> cfgFiles) {
    std::cout << "Creating table...\n";

    using namespace sqlite_orm;
    auto storage = make_storage(
        "db.sqlite",
        make_table("config_programs",
                   make_column("id", &ConfigProgram::id,
                               primary_key().autoincrement()),
                   make_column("tag", &ConfigProgram::tag),
                   make_column("programName", &ConfigProgram::programName),
                   make_column("configPath", &ConfigProgram::configPath)),
        make_table(
            "config_files",
            make_column("id", &ConfigFile::id, primary_key().autoincrement()),
            make_column("program_id", &ConfigFile::programId),
            make_column("files", &ConfigFile::filePath),
            make_column("last_modified", &ConfigFile::lastModified),
            foreign_key(&ConfigFile::programId)
                .references(&ConfigProgram::id)));

    storage.sync_schema();

    std::cout << "Created db.sqlite!\n";

    // Config cfg{.programName = "kitty",
    //            .configPath = "/home/aditya/.config/kitty/"};

    // ConfigFiles cfgFile1{.filePath = "/home/aditya/.config/kitty/kitty.conf",
    //                      .lastModified = "349822"};
    // ConfigFiles cfgFile2{.filePath =
    //                          "/home/aditya/.config/kitty/kitty.conf.bak",
    //                      .lastModified = "349822"};
    // ConfigFiles cfgFile3{.filePath =
    //                          "/home/aditya/.config/kitty/current-theme.conf",
    //                      .lastModified = "349822"};
    // std::vector<ConfigFiles> cfgFiles{cfgFile1, cfgFile2, cfgFile3};
    //
    insertConfig(storage, cfg, cfgFiles);
}
