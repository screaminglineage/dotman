#include "utils.h"
#include <chrono>
#include <filesystem>

namespace chrono = std::chrono;
namespace fs = std::filesystem;
  
std::time_t getlastWriteTime(fs::directory_entry file) {
  auto time = fs::last_write_time(file);
  return chrono::system_clock::to_time_t(
    chrono::clock_cast<chrono::system_clock>(time));
}
