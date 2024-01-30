#include "database.h"
#include "utils.h"
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace fs = std::filesystem;
using VecSV = std::vector<std::string_view>;

enum class CliCommand {
    Add,
    Sync,
    Error,
};

struct ParseResult {
    CliCommand command{};
    std::string_view tag{"primary"};
    VecSV arguments{};
};

ParseResult parseArguments(VecSV& args) {
    auto result = ParseResult{};
    if (args[0] == "add") {
        result.command = CliCommand::Add;
    } else if (args[0] == "sync") {
        result.command = CliCommand::Sync;
    } else {
        std::cerr << "Error: no subcommand specified. available: [add|sync] \n";
        result.command = CliCommand::Error;
        return result;
    }

    // skipping the first arg which is a subcommand
    for (size_t i = 1; i < args.size(); i++) {
        std::string_view arg = args[i];

        std::string_view option{};
        bool shortOption{};
        // checking for options like `--arg` or `-a`
        if (arg.starts_with('-') && arg.size() >= 2) {
            if (arg[1] == '-') {
                shortOption = false;
                option = arg.substr(2);
            } else {
                shortOption = true;
                option = arg.substr(1);
            }

            // should have at least 1 argument after option
            if (i >= args.size() - 1) {
                std::cerr << std::format(
                    "Error: no argument specified to option `{}`\n", option);
                result.command = CliCommand::Error;
                return result;
            }

            if ((option == "tag" && !shortOption) 
                || (option == "t" && shortOption)) {
                result.tag = args[i + 1];
                i++;
            }
            continue;
        }
        result.arguments.push_back(arg);
    }
    return result;
}

// TODO: add option to also pass in the tags along with the program titles
void addPrograms(Storage& storage, VecSV& programTitles) {
    for (const auto& programTitle : programTitles) {
        auto dirPath{paths::configPath / fs::path{programTitle}};

        ProgramData cfg{.title = std::string{programTitle},
                        .configDir = dirPath};
        if (!fs::exists(dirPath)) {
            std::cerr << "error: " << dirPath << " not found!\n";
            continue;
        }

        // add configs to DB
        if (!insertNewConfig(storage, cfg)) {
            std::cerr << std::format(
                "Config: {} with tag `{}` already exists!\n", cfg.title,
                cfg.tag);
            continue;
        }

        // copy over files into backup dir
        std::cout << std::format("Copying Files (`{}`) ...\n", programTitle);
        auto programBackupPath = paths::backupPath / fs::path{cfg.tag};
        if (!fs::exists(programBackupPath)) {
            fs::create_directory(programBackupPath);
        }
        fs::copy(dirPath, programBackupPath / fs::path{cfg.title},
                 fs::copy_options::recursive |
                     fs::copy_options::update_existing);
    }
}

int main(int argc, char* argv[]) {
    std::string_view program = *argv;
    VecSV args(argv + 1, argv + argc);

    auto argOptions = parseArguments(args);

    if (argOptions.command == CliCommand::Error) {
        return 1;
    }

    auto storage = initDb();

    if (argOptions.command == CliCommand::Add) {
        addPrograms(storage, argOptions.arguments);

    } else if (argOptions.command == CliCommand::Sync) {
        for (const auto& prog : argOptions.arguments) {
            if (!configExists(storage, prog, "primary")) {
                std::cerr << std::format("{}: no such program added: `{}`\n",
                                         program, prog);
                return 1;
            }
            if (!syncFiles(storage, getProgramId(storage, prog, "primary"))) {
                std::cerr << std::format(
                    "{}: backup directory not found for `{}`\n", program, prog);
                return 1;
            }
        }
    }

    return 0;
}
