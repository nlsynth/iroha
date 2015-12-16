#include "opt/optimizer.h"

namespace iroha {

Optimizer::Optimizer(IDesign *design) : design_(design) {
}

bool Optimizer::ApplyPhase(const string &name) {
  return true;
}

}  // namespace iroha
