#include <algorithm>
#include <string>

#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
  long h = seconds / 3600;
  int m = (seconds % 3600) / 60;
  int s = seconds % 60;
  char buffer[std::max<int>((std::to_string(h)).length(), 2) + 4];
  sprintf(buffer, "%02ld:%02d:%02d", h, m, s);
  string result(buffer);
  return result;
}
