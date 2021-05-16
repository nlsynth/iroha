// -*- C++ -*-
#ifndef _iroha_base_file_h_
#define _iroha_base_file_h_

#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace iroha {

class File {
 public:
  static void SetImportPaths(const vector<string> &paths);
  static istream *OpenFile(const string &s);
  static string BaseName(const string &fn);

  static vector<string> import_paths_;
};

}  // namespace iroha

#endif  // _iroha_base_file_h_
