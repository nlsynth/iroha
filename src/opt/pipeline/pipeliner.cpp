#include "opt/pipeline/pipeliner.h"

#include "design/design_tool.h"
#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "opt/loop/loop_block.h"
#include "opt/optimizer_log.h"

namespace iroha {
namespace opt {
namespace pipeline {

Pipeliner::Pipeliner(ITable *tab, loop::LoopBlock *lb)
    : tab_(tab), lb_(lb), opt_log_(nullptr), prologue_st_(nullptr) {
  opt_log_ = tab->GetModule()->GetDesign()->GetOptimizerLog();
}

bool Pipeliner::Pipeline() {
  lb_->Annotate(opt_log_);
  ostream &os = opt_log_->GetDumpStream();
  int ns = lb_->GetStates().size();
  os << "Pipeliner " << lb_->GetStates().size() << " states, "
     << lb_->GetLoopCount() << " loop <br/>";
  // prepare states.
  prologue_st_ = new IState(tab_);
  tab_->states_.push_back(prologue_st_);
  int plen = ns + (ns - 1);
  for (int i = 0; i < plen; ++i) {
    IState *st = new IState(tab_);
    pipeline_st_.push_back(st);
    tab_->states_.push_back(st);
  }
  for (int i = 0; i < ns; ++i) {
    for (int j = 0; j <= i; ++j) {
      PlaceState(i, j);
    }
  }
  for (int i = 1; i < ns; ++i) {
    for (int j = i; j < ns; ++j) {
      PlaceState(i + ns - 1, j);
    }
  }
  SetupCounter();
  ConnectPipelineState();
  ConnectPipeline();
  return true;
}

void Pipeliner::PlaceState(int pidx, int lidx) {
  IState *pst = pipeline_st_[pidx];
  ostream &os = opt_log_->State(pst);
  os << "[" << lidx << "]";
  IState *lst = lb_->GetStates().at(lidx);
  for (IInsn *insn : lst->insns_) {
    IResource *res = insn->GetResource();
    if (resource::IsTransition(*(res->GetClass()))) {
      continue;
    }
    IInsn *new_insn = new IInsn(res);
    new_insn->inputs_ = insn->inputs_;
    new_insn->outputs_ = insn->outputs_;
    pst->insns_.push_back(new_insn);
  }
}

void Pipeliner::ConnectPipelineState() {
  DesignTool::AddNextState(prologue_st_, pipeline_st_[0]);
  for (int i = 0; i < pipeline_st_.size() - 1; ++i) {
    IState *cur = pipeline_st_[i];
    IState *next = pipeline_st_[i + 1];
    DesignTool::AddNextState(cur, next);
  }
}

void Pipeliner::ConnectPipeline() {
  // To entry.
  IState *is = lb_->GetEntryAssignState();
  IInsn *einsn = DesignUtil::GetTransitionInsn(is);
  einsn->target_states_[0] = prologue_st_;
  // From last.
  IState *lst = pipeline_st_[pipeline_st_.size() - 1];
  IInsn *linsn = DesignUtil::GetTransitionInsn(lst);
  linsn->target_states_.push_back(lb_->GetExitState());
}

void Pipeliner::SetupCounter() {
  IRegister *orig_counter = lb_->GetRegister();
  IRegister *counter0 = new IRegister(tab_, orig_counter->GetName() + "ps0");
  counter0->value_type_ = orig_counter->value_type_;
  tab_->registers_.push_back(counter0);
  // WIP. Also populate the counter for later stages.
  counters_.push_back(counter0);
  IInsn *insn = new IInsn(DesignUtil::FindAssignResource(tab_));
  insn->inputs_.push_back(orig_counter);
  insn->outputs_.push_back(counter0);
  prologue_st_->insns_.push_back(insn);
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
