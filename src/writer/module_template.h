// -*- C++ -*-
#ifndef _writer_module_template_h_
#define _writer_module_template_h_

#include "iroha/common.h"

#include <sstream>
#include <map>

namespace iroha {

class ModuleTemplate {
public:
  ostream &GetStream(const string &point);
  string GetContents(const string &point);

private:
  map<string, ostringstream> streams_;
};

}  // namespace iroha

#endif  // _writer_module_template_h_
