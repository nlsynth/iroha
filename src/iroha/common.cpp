#include "iroha/common.h"

#include <sstream>

namespace iroha {

string Util::Itoa(int i) {
  stringstream ss;
  ss << i;
  return ss.str();
}

int Util::Atoi(const string &str) {
  stringstream ss;
  ss << str;
  int i = 0;
  ss >> i;
  return i;
}

}  // namespace iroha
