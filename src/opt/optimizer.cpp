#include "opt/optimizer.h"

#include "opt/phase.h"

namespace iroha {

vector<Phase *> Optimizer::phases_;

Optimizer::Optimizer(IDesign *design) : design_(design) {
}

void Optimizer::Init() {
}

void Optimizer::RegisterPhase(Phase *phase) {
  phases_.push_back(phase);
}

bool Optimizer::ApplyPhase(const string &name) {
  return true;
}

}  // namespace iroha
