#include "writer/cxx/module.h"

namespace iroha {
namespace writer {
namespace cxx {

Module::Module(const IModule *i_mod) : i_mod_(i_mod) {
}

void Module::Write(ostream &os) {
}

}  // namespace cxx
}  // namespace writer
}  // namespace iroha
