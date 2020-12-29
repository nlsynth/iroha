#include "opt/pipeline/insn_condition.h"

#include "opt/loop/loop_block.h"

namespace iroha {
namespace opt {
namespace pipeline {

InsnCondition::InsnCondition(loop::LoopBlock *lb)
    : tab_(lb->GetTable()), lb_(lb) {}

bool InsnCondition::Build(OptimizerLog *log) { return true; }

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha