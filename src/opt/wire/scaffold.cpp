#include "opt/wire/scaffold.h"

#include "design/design_tool.h"
#include "design/design_util.h"
#include "iroha/stl_util.h"
#include "iroha/resource_class.h"
#include "opt/bb_set.h"
#include "opt/data_flow.h"
#include "opt/debug_annotation.h"

namespace iroha {
namespace opt {
namespace wire {

Scaffold::Scaffold(ITable *table, DebugAnnotation *annotation)
  : table_(table), annotation_(annotation) {
}

Scaffold::~Scaffold() {
  STLDeleteSecondElements(&per_insn_map_);
}

void Scaffold::SetUp() {
  assign_ = DesignTool::GetOneResource(table_, resource::kSet);
  transition_ = DesignUtil::FindTransitionResource(table_);
  bset_.reset(BBSet::Create(table_, annotation_));
  data_flow_.reset(DataFlow::Create(bset_.get(), annotation_));
  if (annotation_ != nullptr) {
    annotation_->DumpIntermediateTable(table_);
  }
}

Scaffold::PerInsn *Scaffold::GetPerInsn(IInsn *insn) {
  PerInsn *pi = per_insn_map_[insn];
  if (pi == nullptr) {
    pi = new PerInsn;
    per_insn_map_[insn] = pi;
  }
  return pi;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
