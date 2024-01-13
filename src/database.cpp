#include "../include/sqlite_orm/sqlite_orm.h"
#include <iostream>
#include <memory>

struct User {
    int id;
    std::string firstName;
    std::string lastName;
    int birthDate;
    std::unique_ptr<std::string> imageUrl;
    int typeId;
};

struct UserType {
    int id;
    std::string name;
};

void createDb() {
    std::cout << "Creating table...\n";

    using namespace sqlite_orm;
    auto storage = make_storage(
        "db.sqlite",
        make_table("users",
                   make_column("id", &User::id, primary_key().autoincrement()),
                   make_column("first_name", &User::firstName),
                   make_column("last_name", &User::lastName),
                   make_column("birth_date", &User::birthDate),
                   make_column("image_url", &User::imageUrl),
                   make_column("type_id", &User::typeId)),
        make_table(
            "user_types",
            make_column("id", &UserType::id, primary_key().autoincrement()),
            make_column("name", &UserType::name,
                        default_value("name_placeholder"))));
    storage.sync_schema();
    std::cout << "Created db.sqlite!\n";

    User user{-1,
              "Jonh",
              "Doe",
              664416000,
              std::make_unique<std::string>("url_to_heaven"),
              3};

    auto insertedId = storage.insert(user);
    std::cout << "insertedId = " << insertedId << std::endl; //  insertedId = 8
    user.id = insertedId;

    User secondUser{-1, "Alice", "Inwonder", 831168000, {}, 2};
    insertedId = storage.insert(secondUser);
    secondUser.id = insertedId;
}
