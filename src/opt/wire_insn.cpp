#include "opt/wire_insn.h"

#include "opt/bb_set.h"
#include "opt/data_flow.h"

namespace iroha {
namespace opt {

WireInsnPhase::~WireInsnPhase() {
}

Phase *WireInsnPhase::Create() {
  return new WireInsnPhase();
}

bool WireInsnPhase::ApplyForTable(ITable *table) {
  WireInsn wire_insn(table, annotation_);
  return wire_insn.Perform();
}

WireInsn::WireInsn(ITable *table, DebugAnnotation *annotation)
  : table_(table), annotation_(annotation), bbs_(nullptr), data_flow_(nullptr) {
}

WireInsn::~WireInsn() {
  delete bbs_;
  delete data_flow_;
}

bool WireInsn::Perform() {
  bbs_ = BBSet::Create(table_, annotation_);
  data_flow_ = DataFlow::Create(bbs_, annotation_);
  return true;
}

}  // namespace opt
}  // namespace iroha
