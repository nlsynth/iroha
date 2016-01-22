#include "writer/module_template.h"

namespace iroha {
namespace writer {

ostream &ModuleTemplate::GetStream(const string &point) {
  return streams_[point];
}

string ModuleTemplate::GetContents(const string &point) {
  return streams_[point].str();
}

}  // namespace writer
}  // namespace iroha
