// -*- C++ -*-
#ifndef _iroha_base_file_h_
#define _iroha_base_file_h_

#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

namespace iroha {

class File {
 public:
  static void SetImportPaths(const vector<string> &paths);
  static istream *OpenFile(const string &s);
  static string BaseName(const string &fn);
  static void RegisterFile(const string &fn, const string &data);

 private:
  static bool UseSearchPath(const string &fn);

  static vector<string> import_paths_;
  static map<string, string> file_data_;
};

}  // namespace iroha

#endif  // _iroha_base_file_h_
