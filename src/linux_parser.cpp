#include <dirent.h>
#include <iomanip>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  float mem_total, mem_free;
  string line, key, value;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal") {
          mem_total = std::stof(value);
        }
        if (key == "MemFree") {
          mem_free = std::stof(value);
        }
      }
    }
    return (mem_total - mem_free) / mem_total;
  }
  return 0.0;
}

long LinuxParser::UpTime() {
  string line, value;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> value;
    return std::stol(value);
  }
  return 0;
}

long LinuxParser::Jiffies() {
  // jiffies = user_ - guest_ + nice_ - guest_nice_ + system_ + irq_ + softirq_ + idle_ + iowait_ + steal_ + guest_ + guest_nice_
  // jiffies = user_ + nice_ + system_ + irq_ + softirq_ + idle_ + iowait_ + steal_
  std::vector<string> values = CpuUtilization();
  if (values.size() > kSteal_ + 1) {
    long result = 0;
    for (int i = kUser_; i <= kSteal_; i++) {
      result += std::stol(values[i]);
    }
    return result;
  }
  return 0;
}

long LinuxParser::ActiveJiffies(int pid) {
  string line, key, value;
  std::vector<std::string> values;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::stringstream sstream(line);
    while (std::getline(sstream, value, ' ')) {
      values.push_back(value);
    }
    if (values.size() > 16) {
      return std::stol(values[13]) +
        std::stol(values[14]) +
        std::stol(values[15]) +
        std::stol(values[16]);
    }
  }
  return 0;
}

long LinuxParser::ActiveJiffies() {
  return Jiffies() - IdleJiffies();
}

long LinuxParser::IdleJiffies() {
  // idlejiffies = idle_ + iowait_
  std::vector<string> values = CpuUtilization();
  if (values.size() > kIOwait_ + 1) {
    long result = 0;
    for (int i = kIdle_; i <= kIOwait_; i++) {
      result += std::stol(values[i]);
    }
    return result;
  }
  return 0;
}

vector<string> LinuxParser::CpuUtilization() {
  string line, key, value;
  std::vector<std::string> values;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> value; // skips first value -- cpu
    while (linestream >> value) {
      values.push_back(value);
    }
    return values;
  }
  return {};
}

int LinuxParser::TotalProcesses() {
  string line, key, value;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          return std::stoi(value);
        }
      }
    }
  }
  return 0;
}

int LinuxParser::RunningProcesses() { string line, key, value;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          return std::stoi(value);
        }
      }
    }
  }
  return 0;
}

string LinuxParser::Command(int pid) {
  string line;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    return line;
  }
  return string();
}

string LinuxParser::Ram(int pid) {
  string line, key, value;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize") {
          std::stringstream sstream;
          sstream << std::fixed << std::setprecision(2) << std::stof(value) / 1024;
          return sstream.str();
        }
      }
    }
  }
  return string();
}

string LinuxParser::Uid(int pid) {
  string line, key, value;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid") {
          return value;
        }
      }
    }
  }
  return string();
}

string LinuxParser::User(int pid) {
  string line, key, _, uid;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> _ >> uid) {
        if (uid == Uid(pid)) {
          return key;
        }
      }
    }
  }
  return string();
}

long LinuxParser::UpTime(int pid) {
  string line, value;
  std::vector<std::string> values;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      values.push_back(value);
    }
    if (values.size() > 21) {
      return UpTime() - std::stol(values[21]) / sysconf(_SC_CLK_TCK);
    }
  }
  return 0;
}
