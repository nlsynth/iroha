#include "iroha/base/file.h"

#include <string.h>

#include <fstream>
#include <sstream>

namespace iroha {

vector<string> File::import_paths_;
map<string, string> File::file_data_;

void File::SetImportPaths(const vector<string> &paths) {
  import_paths_ = paths;
}

istream *File::OpenFile(const string &s) {
  ifstream *ifs;
  if (UseSearchPath(s)) {
    for (const string &p : import_paths_) {
      ifs = new ifstream(p + "/" + s);
      if (!ifs->fail()) {
        return ifs;
      }
      delete ifs;
    }
  }
  ifs = new ifstream(s);
  if (!ifs->fail()) {
    return ifs;
  }
  delete ifs;
  auto it = file_data_.find(s);
  if (it != file_data_.end()) {
    return new istringstream(it->second);
  }
  return nullptr;
}

bool File::UseSearchPath(const string &fn) {
  // Don't use search path for absolute path or dot file.
  if (fn.find("/") == 0 || fn.find(".") == 0) {
    return false;
  }
  return true;
}

string File::BaseName(const string &fn) {
  const char *p = strrchr(fn.c_str(), '/');
  if (!p) {
    return fn;
  }
  ++p;
  return string(p);
}

void File::RegisterFile(const string &fn, const string &data) {
  file_data_[fn] = data;
}

}  // namespace iroha
