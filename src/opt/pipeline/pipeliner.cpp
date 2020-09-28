#include "opt/pipeline/pipeliner.h"

#include "design/design_tool.h"
#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "opt/loop/loop_block.h"
#include "opt/optimizer_log.h"
#include "opt/pipeline/stage_scheduler.h"

namespace iroha {
namespace opt {
namespace pipeline {

Pipeliner::Pipeliner(ITable *tab, StageScheduler *ssch)
    : tab_(tab),
      ssch_(ssch),
      lb_(ssch->GetLoop()),
      interval_(ssch_->GetInterval()),
      opt_log_(nullptr),
      prologue_st_(nullptr) {
  opt_log_ = tab->GetModule()->GetDesign()->GetOptimizerLog();
}

bool Pipeliner::Pipeline() {
  lb_->Annotate(opt_log_);
  ostream &os = opt_log_->GetDumpStream();
  os << "Pipeliner " << ssch_->GetMacroStageCount() << " states, "
     << lb_->GetLoopCount() << " loop, interval=" << interval_ << " <br/>";
  // prepare states.
  prologue_st_ = new IState(tab_);
  tab_->states_.push_back(prologue_st_);
  int plen = ssch_->GetPipelineStageLength();
  for (int i = 0; i < plen; ++i) {
    IState *st = new IState(tab_);
    pipeline_stages_.push_back(st);
    tab_->states_.push_back(st);
  }
  // s0
  // s0 s1
  // s0 s1 s2
  // .. ..
  // s0 .. .. s{n-1}
  int ns = ssch_->GetMacroStageCount();
  for (int i = 0; i < ns; ++i) {
    for (int j = 0; j <= i; ++j) {
      PlaceState(i, j);
    }
  }
  // -- s1 s2 .. s{n-1}
  // -- -- s2 .. s{n-1}
  //             ..
  //             s{n-1}
  for (int i = 1; i < ns; ++i) {
    for (int j = i; j < ns; ++j) {
      PlaceState(i + ns - 1, j);
    }
  }
  SetupCounter();
  SetupCounterIncrement();
  UpdateCounterRead();
  ConnectPipelineState();
  SetupExit();
  ConnectPipeline();
  return true;
}

void Pipeliner::PlaceState(int pidx, int lidx) {
  IState *pst = pipeline_stages_[pidx * interval_];
  ostream &os = opt_log_->State(pst);
  os << "[" << lidx << "]";
  MacroStage &ms = ssch_->GetMacroStage(lidx);
  for (IInsn *insn : ms.insns_) {
    IResource *res = insn->GetResource();
    if (resource::IsTransition(*(res->GetClass()))) {
      continue;
    }
    IInsn *new_insn = new IInsn(res);
    new_insn->inputs_ = insn->inputs_;
    new_insn->outputs_ = insn->outputs_;
    pst->insns_.push_back(new_insn);
    insn_to_stage_[new_insn] = lidx;
  }
}

void Pipeliner::UpdateCounterRead() {
  IRegister *counter = lb_->GetRegister();
  for (IState *st : pipeline_stages_) {
    for (IInsn *insn : st->insns_) {
      auto it = insn_to_stage_.find(insn);
      if (it == insn_to_stage_.end()) {
        continue;
      }
      int stage = it->second;
      for (int i = 0; i < insn->inputs_.size(); ++i) {
        if (insn->inputs_[i] == counter) {
          insn->inputs_[i] = counters_[stage];
        }
      }
    }
  }
}

void Pipeliner::ConnectPipelineState() {
  DesignTool::AddNextState(prologue_st_, pipeline_stages_[0]);
  for (int i = 0; i < pipeline_stages_.size() - 1; ++i) {
    IState *cur = pipeline_stages_[i];
    IState *next = pipeline_stages_[i + 1];
    DesignTool::AddNextState(cur, next);
  }
}

void Pipeliner::ConnectPipeline() {
  // To entry.
  IState *is = lb_->GetEntryAssignState();
  IInsn *einsn = DesignUtil::GetTransitionInsn(is);
  einsn->target_states_[0] = prologue_st_;
  // From last.
  IState *lst = pipeline_stages_[pipeline_stages_.size() - 1];
  IInsn *linsn = DesignUtil::GetTransitionInsn(lst);
  if (pipeline_stages_.size() == 1) {
    // exit, current.
    linsn->target_states_.push_back(linsn->target_states_[0]);
    linsn->target_states_[0] = lb_->GetExitState();
  } else {
    linsn->target_states_.push_back(lb_->GetExitState());
  }
}

void Pipeliner::SetupCounter() {
  int llen = ssch_->GetMacroStageCount();
  IRegister *orig_counter = lb_->GetRegister();
  for (int i = 0; i < llen; ++i) {
    IRegister *counter = new IRegister(tab_, RegName("ps", i));
    counter->value_type_ = orig_counter->value_type_;
    tab_->registers_.push_back(counter);
    counters_.push_back(counter);
    IRegister *counter_wire = new IRegister(tab_, RegName("psw", i));
    counter_wire->SetStateLocal(true);
    counter_wire->value_type_ = orig_counter->value_type_;
    tab_->registers_.push_back(counter_wire);
    counter_wires_.push_back(counter_wire);
  }
  IResource *assign = DesignUtil::FindAssignResource(tab_);
  for (int i = 0; i < llen * 2 - 1; ++i) {
    IState *st = pipeline_stages_[i * interval_ + (interval_ - 1)];
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
  int llen = ssch_->GetMacroStageCount();
  int cwidth = counter0->value_type_.GetWidth();
  IResource *adder = DesignUtil::CreateResource(tab_, resource::kAdd);
  adder->input_types_.push_back(counter0->value_type_);
  adder->input_types_.push_back(counter0->value_type_);
  adder->output_types_.push_back(counter0->value_type_);
  IResource *assign = DesignUtil::FindAssignResource(tab_);
  IRegister *one = DesignTool::AllocConstNum(tab_, cwidth, 1);
  for (int i = 0; i < llen; ++i) {
    IInsn *add_insn = new IInsn(adder);
    add_insn->inputs_.push_back(counter0);
    add_insn->inputs_.push_back(one);
    add_insn->outputs_.push_back(counter_wires_[i]);
    pipeline_stages_[i * interval_ + (interval_ - 1)]->insns_.push_back(
        add_insn);
    IInsn *wire_to_reg = new IInsn(assign);
    wire_to_reg->inputs_.push_back(counter_wires_[i]);
    wire_to_reg->outputs_.push_back(counter0);
    pipeline_stages_[i * interval_ + (interval_ - 1)]->insns_.push_back(
        wire_to_reg);
  }
}

void Pipeliner::SetupExit() {
  int llen = ssch_->GetMacroStageCount();
  IState *st = pipeline_stages_[(llen - 1) * interval_ + (interval_ - 1)];
  IInsn *tr_insn = DesignUtil::GetTransitionInsn(st);
  // next, current
  tr_insn->target_states_.push_back(st);
  IRegister *cond = new IRegister(tab_, RegName("cond_ps", 0));
  cond->value_type_.SetWidth(0);
  cond->SetStateLocal(true);
  tab_->registers_.push_back(cond);
  tr_insn->inputs_.push_back(cond);

  IRegister *counter0 = counters_[0];
  int cwidth = counter0->value_type_.GetWidth();
  IRegister *max = DesignTool::AllocConstNum(tab_, cwidth, lb_->GetLoopCount());
  IResource *comparator = DesignUtil::CreateResource(tab_, resource::kGt);
  comparator->input_types_.push_back(counter0->value_type_);
  comparator->input_types_.push_back(counter0->value_type_);
  IValueType o;
  o.SetWidth(0);
  comparator->output_types_.push_back(o);
  IInsn *compare = new IInsn(comparator);
  compare->outputs_.push_back(cond);
  compare->inputs_.push_back(max);
  compare->inputs_.push_back(counter_wires_[llen - 1]);
  st->insns_.push_back(compare);
}

string Pipeliner::RegName(const string &base, int index) {
  IRegister *orig_counter = lb_->GetRegister();
  return orig_counter->GetName() + base + Util::Itoa(index);
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
