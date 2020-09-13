#include "opt/pipeline/pipeliner.h"

namespace iroha {
namespace opt {
namespace pipeline {

Pipeliner::Pipeliner(ITable *tab, loop::LoopBlock *lb) : tab_(tab), lb_(lb) {}

bool Pipeliner::Pipeline() { return true; }

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
