#pragma once
#include <chrono>
#include <filesystem>

std::time_t getlastWriteTime(std::filesystem::directory_entry file);
