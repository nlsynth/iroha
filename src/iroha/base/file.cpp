#include "iroha/base/file.h"

#include <string.h>

#include <fstream>

namespace iroha {

vector<string> File::import_paths_;

void File::SetImportPaths(const vector<string> &paths) {
  import_paths_ = paths;
}

istream *File::OpenFile(const string &s) {
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

string File::BaseName(const string &fn) {
  const char *p = strrchr(fn.c_str(), '/');
  if (!p) {
    return fn;
  }
  ++p;
  return string(p);
}

}  // namespace iroha
