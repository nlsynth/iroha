#include "iroha/common.h"

#include <string.h>
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

void Util::SplitStringUsing(const string &str, const char *delim,
			    vector<string> *output) {
  int cur = 0;
  while (cur < str.size()) {
    int len = 0;
    while (cur + len < str.size()) {
      char c = str.c_str()[cur + len];
      if (strchr(delim, c) != nullptr) {
	break;
      }
      ++len;
    }
    output->push_back(str.substr(cur, len));
    cur += len + 1;
  }
}

}  // namespace iroha
