#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int p) : pid_(p) {
  cpu_ = CpuUtilization();
};

int Process::Pid() { return pid_; }

float Process::CpuUtilization() {
  float totaltime = (float)LinuxParser::ActiveJiffies(pid_);
  float seconds = (float)LinuxParser::UpTime(pid_);
  return totaltime / sysconf(_SC_CLK_TCK) / seconds;
}

string Process::Command() {
  return LinuxParser::Command(pid_);
}

string Process::Ram() { return LinuxParser::Ram(pid_); }

string Process::User() { return LinuxParser::User(pid_); }

long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

bool Process::operator<(Process const& a) const {
  return cpu_ > a.cpu_;
}
