#include "iroha/common.h"

#include <algorithm>
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

string Util::ToLower(const string &s) {
  string l(s);
  std::transform(l.begin(), l.end(), l.begin(), ::tolower);
  return l;
}

string Util::Join(const vector<string> &v, const string &sep) {
  string r;
  bool is_first = true;
  for (auto &s : v) {
    if (!is_first) {
      r += sep;
    }
    r += s;
    is_first = false;
  }
  return r;
}

}  // namespace iroha
