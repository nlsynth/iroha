// -*- C++ -*-
#ifndef _writer_module_template_h_
#define _writer_module_template_h_

#include "iroha/common.h"

#include <sstream>
#include <map>

namespace iroha {
namespace writer {

class ModuleTemplate {
public:
  ostream &GetStream(const string &point);
  string GetContents(const string &point);
  void Clear(const string &point);

private:
  map<string, ostringstream> streams_;
};

}  // namespace writer
}  // namespace iroha

#endif  // _writer_module_template_h_
