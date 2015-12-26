#include "opt/phase.h"

namespace iroha {

Phase::~Phase() {
}

const string &Phase::GetName() const {
  return "null";
}

void Phase::Register(Phase *phase) {
  Optimizer::RegisterPhase(phase);
}

bool Phase::ApplyForDesign(IDesign *design) {
  return true;
}

bool Phase::ApplyForModule(IModule *module) {
  return true;
}

bool Phase::ApplyForTable(ITable *table) {
  return true;
}

}  // iroha
