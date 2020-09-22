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
    : tab_(tab),
      lb_(lb),
      interval_(1),
      opt_log_(nullptr),
      prologue_st_(nullptr) {
  opt_log_ = tab->GetModule()->GetDesign()->GetOptimizerLog();
}

bool Pipeliner::Pipeline() {
  lb_->Annotate(opt_log_);
  ostream &os = opt_log_->GetDumpStream();
  int ns = lb_->GetStates().size();
  os << "Pipeliner " << lb_->GetStates().size() << " states, "
     << lb_->GetLoopCount() << " loop, interval=" << interval_ << " <br/>";
  // prepare states.
  prologue_st_ = new IState(tab_);
  tab_->states_.push_back(prologue_st_);
  int plen = (ns + (ns - 1)) * interval_;
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
  SetupCounterIncrement();
  ConnectPipelineState();
  SetupExit();
  ConnectPipeline();
  return true;
}

void Pipeliner::PlaceState(int pidx, int lidx) {
  IState *pst = pipeline_st_[pidx * interval_];
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
  int llen = lb_->GetStates().size();
  IRegister *orig_counter = lb_->GetRegister();
  for (int i = 0; i < llen; ++i) {
    IRegister *counter =
        new IRegister(tab_, orig_counter->GetName() + "ps" + Util::Itoa(i));
    counter->value_type_ = orig_counter->value_type_;
    tab_->registers_.push_back(counter);
    counters_.push_back(counter);
  }
  IResource *assign = DesignUtil::FindAssignResource(tab_);
  for (int i = 0; i < llen * 2 - 1; ++i) {
    IState *st = pipeline_st_[i * interval_ + (interval_ - 1)];
    int start = 0;
    if (i >= llen) {
      start = i - llen + 1;
    }
    for (int j = start; j <= i; ++j) {
      if (j + 1 >= counters_.size()) {
        continue;
      }
      IInsn *insn = new IInsn(assign);
      st->insns_.push_back(insn);
      insn->inputs_.push_back(counters_[j]);
      insn->outputs_.push_back(counters_[j + 1]);
    }
  }
}

void Pipeliner::SetupCounterIncrement() {
  // Assign at the prologue.
  IInsn *insn = new IInsn(DesignUtil::FindAssignResource(tab_));
  IRegister *orig_counter = lb_->GetRegister();
  insn->inputs_.push_back(orig_counter);
  IRegister *counter0 = counters_[0];
  insn->outputs_.push_back(counter0);
  prologue_st_->insns_.push_back(insn);
  // Increment count0.
  int llen = lb_->GetStates().size();
  int cwidth = counter0->value_type_.GetWidth();
  IResource *adder = DesignUtil::CreateResource(tab_, resource::kAdd);
  adder->input_types_.push_back(counter0->value_type_);
  adder->input_types_.push_back(counter0->value_type_);
  adder->output_types_.push_back(counter0->value_type_);
  IRegister *one = DesignTool::AllocConstNum(tab_, cwidth, 1);
  for (int i = 0; i < llen; ++i) {
    IInsn *add_insn = new IInsn(adder);
    add_insn->inputs_.push_back(counter0);
    add_insn->inputs_.push_back(one);
    add_insn->outputs_.push_back(counter0);
    pipeline_st_[i * interval_ + (interval_ - 1)]->insns_.push_back(add_insn);
  }
}

void Pipeliner::SetupExit() {
  int llen = lb_->GetStates().size();
  IState *st = pipeline_st_[(llen - 1) * interval_ + (interval_ - 1)];
  IInsn *tr_insn = DesignUtil::GetTransitionInsn(st);
  tr_insn->target_states_.push_back(st);
  IRegister *cond = new IRegister(tab_, "cond_ps0");
  tab_->registers_.push_back(cond);
  tr_insn->inputs_.push_back(cond);

  IRegister *counter0 = counters_[0];
  int cwidth = counter0->value_type_.GetWidth();
  IRegister *max = DesignTool::AllocConstNum(tab_, cwidth, 1);
  IResource *comparator = DesignUtil::CreateResource(tab_, resource::kGt);
  comparator->input_types_.push_back(counter0->value_type_);
  comparator->input_types_.push_back(counter0->value_type_);
  IValueType o;
  o.SetWidth(0);
  comparator->output_types_.push_back(o);
  IInsn *compare = new IInsn(comparator);
  compare->outputs_.push_back(cond);
  compare->inputs_.push_back(max);
  compare->inputs_.push_back(counter0);
  st->insns_.push_back(compare);
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
