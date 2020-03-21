#include "opt/unroll/unroll_phase.h"

namespace iroha {
namespace opt {
namespace unroll {

UnrollPhase::~UnrollPhase() {
}

Phase *UnrollPhase::Create() {
  return new UnrollPhase();
}

}  // namespace unroll
}  // namespace opt
}  // namespace iroha
