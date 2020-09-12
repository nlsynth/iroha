#include "opt/clean/pseudo_resource.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"

namespace iroha {
namespace opt {
namespace clean {

CleanPseudoResourcePass::~CleanPseudoResourcePass() {}

Pass *CleanPseudoResourcePass::Create() {
  return new CleanPseudoResourcePass();
}

bool CleanPseudoResourcePass::ApplyForTable(const string &key, ITable *table) {
  for (IState *st : table->states_) {
    vector<IInsn *> real_insns;
    for (IInsn *insn : st->insns_) {
      if (!resource::IsPseudo(*(insn->GetResource()->GetClass()))) {
        real_insns.push_back(insn);
      }
    }
    st->insns_ = real_insns;
  }
  vector<IResource *> real_resources;
  for (IResource *res : table->resources_) {
    if (!resource::IsPseudo(*(res->GetClass()))) {
      real_resources.push_back(res);
    }
  }
  table->resources_ = real_resources;
  return true;
}

}  // namespace clean
}  // namespace opt
}  // namespace iroha
