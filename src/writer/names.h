//  -*- C++ -*-
//
// This class manages names in output file in order to avoid conflits
// between names or against reserved keywords.
// Basic strategies are:
// * Avoid names with out '_' (underscore), since they are likely be reserved.
// * Use the substring before first '_' as a prefix and add prefix to the
//   original name, if necessary.
//
#ifndef _writer_names_h_
#define _writer_names_h_

#include "iroha/common.h"

#include <set>

namespace iroha {
namespace writer {

class Names {
public:
  void ReservePrefix(const string &prefix);
  string GetName(const IRegister &reg);

private:
  string GetPrefix(string &s);

  set<string> prefixes_;
};

}  // namespace writer
}  // namespace iroha

#endif  // _writer_names_h_
