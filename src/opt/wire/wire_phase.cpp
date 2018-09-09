#include "opt/wire/wire_phase.h"

#include "opt/wire/simple_shrink.h"
#include "opt/wire/wire.h"

namespace iroha {
namespace opt {
namespace wire {

SimpleShrinkPhase::~SimpleShrinkPhase() {
}

Phase *SimpleShrinkPhase::Create() {
  return new SimpleShrinkPhase();
}

bool SimpleShrinkPhase::ApplyForTable(const string &key, ITable *table) {
  SimpleShrink simple_shrink(table, annotation_);
  return simple_shrink.Perform();
}

WirePhase::~WirePhase() {
}

Phase *WirePhase::Create() {
  return new WirePhase();
}

bool WirePhase::ApplyForTable(const string &key, ITable *table) {
  Wire wire(table, annotation_);
  return wire.Perform();
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
