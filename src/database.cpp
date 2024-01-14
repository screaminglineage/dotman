#include "../include/sqlite_orm/sqlite_orm.h"
#include <iostream>
#include <string>

struct Config {
    // int uuid;
    int id;
    std::string tag;
    std::string programName;
    std::string configPath;
};

struct ConfigFiles {
    // int uuid;
    int id;
    int programId;
    std::string filePath;
};

void createDb() {
    std::cout << "Creating table...\n";

    using namespace sqlite_orm;
    auto storage = make_storage(
        "db.sqlite",
        make_table(
            "config_programs",
            make_column("id", &Config::id, primary_key().autoincrement()),
            make_column("tag", &Config::tag),
            make_column("programName", &Config::programName),
            make_column("configPath", &Config::configPath)),
        make_table(
            "config_files",
            make_column("id", &ConfigFiles::id, primary_key().autoincrement()),
            make_column("program_id", &ConfigFiles::programId),
            make_column("files", &ConfigFiles::filePath),
            foreign_key(&ConfigFiles::programId).references(&Config::id)));
    storage.sync_schema();

    std::cout << "Created db.sqlite!\n";

    Config cfg{-1, "primary", "kitty", "/home/aditya/.config/kitty/"};
    auto insertedId = storage.insert(cfg);
    std::cout << "insertedId = " << insertedId << std::endl;
    cfg.id = insertedId;

    Config cfg2{-1, "primary", "rofi", "/home/aditya/.config/rofi/"};
    insertedId = storage.insert(cfg2);
    std::cout << "insertedId = " << insertedId << std::endl;
    cfg.id = insertedId;
}
