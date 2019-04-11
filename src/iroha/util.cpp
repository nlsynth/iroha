#include "iroha/util.h"

#include <algorithm>
#include <string.h>
#include <fstream>
#include <sstream>

namespace iroha {

vector<string> Util::import_paths_;

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

uint64_t Util::AtoULL(const string &str) {
  stringstream ss;
  ss << str;
  uint64_t i = 0;
  ss >> i;
  return i;
}

string Util::ULLtoA(uint64_t u) {
  stringstream ss;
  ss << u;
  return ss.str();
}

int Util::Log2(uint64_t u) {
  int r = 0;
  while (u > 1) {
    u >>= 1;
    ++r;
  }
  return r;
}

bool Util::IsInteger(const string &a) {
  return Atoi(a) != 0 || a == "0";
}

void Util::SetImportPaths(const vector<string> &paths) {
  import_paths_ = paths;
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

istream *Util::OpenFile(const string &s) {
  // TODO: Search import_paths_ first.
  ifstream *ifs = new ifstream(s);
  if (!ifs->fail()) {
    return ifs;
  }
  delete ifs;
  if (s.find("/") == 0 || s.find(".") == 0) {
    return nullptr;
  }
  for (const string &p : import_paths_) {
    ifs = new ifstream(p + "/" + s);
    if (!ifs->fail()) {
      return ifs;
    }
    delete ifs;
  }
  return nullptr;
}

string Util::BaseName(const string &fn) {
  const char *p = strrchr(fn.c_str(), '/');
  if (!p) {
    return fn;
  }
  ++p;
  return string(p);
}

}  // namespace iroha
