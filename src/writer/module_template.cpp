#include "writer/module_template.h"

namespace iroha {

ostream &ModuleTemplate::GetStream(const string &point) {
  return streams_[point];
}

string ModuleTemplate::GetContents(const string &point) {
  return streams_[point].str();
}

}  // namespace iroha
