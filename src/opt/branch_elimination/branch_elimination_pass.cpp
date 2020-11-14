#include "opt/branch_elimination/branch_elimination_pass.h"

namespace iroha {
namespace opt {
namespace branch_elimination {

BranchElminationPass::~BranchElminationPass() {}

Pass *BranchElminationPass::Create() { return new BranchElminationPass(); }

bool BranchElminationPass::ApplyForTable(const string &key, ITable *table) {
  return true;
}

}  // namespace branch_elimination
}  // namespace opt
}  // namespace iroha
