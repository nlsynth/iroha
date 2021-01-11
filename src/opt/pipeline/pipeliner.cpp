#include "opt/pipeline/pipeliner.h"

#include "design/design_tool.h"
#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "opt/loop/loop_block.h"
#include "opt/optimizer_log.h"
#include "opt/pipeline/insn_condition.h"
#include "opt/pipeline/reg_info.h"
#include "opt/pipeline/scheduled_shape.h"
#include "opt/pipeline/stage_scheduler.h"

namespace iroha {
namespace opt {
namespace pipeline {

Pipeliner::Pipeliner(ITable *tab, StageScheduler *ssch, RegInfo *reg_info,
                     InsnCondition *insn_cond)
    : tab_(tab),
      ssch_(ssch),
      reg_info_(reg_info),
      insn_cond_(insn_cond),
      lb_(ssch->GetLoop()),
      interval_(ssch_->GetInterval()),
      opt_log_(nullptr),
      scheduled_shape_(new ScheduledShape(ssch)),
      prologue_st_(nullptr) {
  opt_log_ = tab->GetModule()->GetDesign()->GetOptimizerLog();
  assign_ = DesignUtil::FindAssignResource(tab_);
}

Pipeliner::~Pipeliner() {}

bool Pipeliner::Pipeline() {
  lb_->Annotate(opt_log_);
  ostream &os = opt_log_->GetDumpStream();
  os << "Pipeliner " << ssch_->GetMacroStageCount() << " states, "
     << lb_->GetLoopCount() << " loop, interval=" << interval_ << " <br/>\n";
  PrepareStates();
  vector<pair<int, int>> loc = scheduled_shape_->GetPipelineLocation();
  for (pair<int, int> &p : loc) {
    int pipeline_macro_stage_index = p.first;
    int loop_macro_stage_index = p.second;
    PlaceState(pipeline_macro_stage_index, loop_macro_stage_index);
  }
  // Write -> Read deps.
  PrepareRegWriteReadPipelineRegs();
  PrepareRegWriteReadPipelineStages();
  UpdateRegWriteReadPipelineAccess();
  // Access to condition regs.
  PrepareInsnCondRegPipelineRegs();
  PrepareInsnCondRegPipelineStages();
  SetInstructionConditions();

  SetupCounter();
  SetupCounterIncrement();
  UpdateCounterRead();
  ConnectPipelineState();
  SetupExit();
  ConnectPipeline();
  return true;
}

void Pipeliner::PrepareStates() {
  prologue_st_ = new IState(tab_);
  tab_->states_.push_back(prologue_st_);
  int plen = ssch_->GetPipelineStageLength();
  for (int i = 0; i < plen; ++i) {
    IState *st = new IState(tab_);
    pipeline_stages_.push_back(st);
    tab_->states_.push_back(st);
  }
}

void Pipeliner::PlaceState(int pipeline_macro_stage_index,
                           int loop_macro_stage_index) {
  MacroStage &ms = ssch_->GetMacroStage(loop_macro_stage_index);
  for (int lst = 0; lst < ms.local_stages_.size(); ++lst) {
    IState *pst =
        pipeline_stages_[pipeline_macro_stage_index * interval_ + lst];
    ostream &os = opt_log_->State(pst);
    os << "[" << loop_macro_stage_index;
    if (ms.local_stages_.size() > 1) {
      os << ":" << lst;
    }
    os << "]";
    StageInsns &si = ms.local_stages_[lst];
    for (IInsn *insn : si.insns_) {
      IResource *res = insn->GetResource();
      if (resource::IsTransition(*(res->GetClass()))) {
        continue;
      }
      IInsn *new_insn = new IInsn(res);
      UpdateRegs(pst, loop_macro_stage_index, false, insn->inputs_,
                 &new_insn->inputs_);
      UpdateRegs(pst, loop_macro_stage_index, true, insn->outputs_,
                 &new_insn->outputs_);
      new_insn->SetOperand(insn->GetOperand());
      pst->insns_.push_back(new_insn);
      insn_to_stage_[new_insn] = loop_macro_stage_index;
    }
  }
}

void Pipeliner::UpdateRegs(IState *pst, int lidx, bool is_output,
                           vector<IRegister *> &src, vector<IRegister *> *dst) {
  IRegister *counter = lb_->GetCounterRegister();
  for (IRegister *reg : src) {
    if (reg->IsStateLocal()) {
      reg = MayUpdateWireReg(pst, reg);
    } else if (counter == reg) {
      // doesn't update.
    } else if (reg->IsNormal() && !is_output) {
      reg = LookupStagedReg(lidx, reg);
    }
    dst->push_back(reg);
  }
}

IRegister *Pipeliner::MayUpdateWireReg(IState *pst, IRegister *reg) {
  auto key = make_tuple(pst, reg);
  auto it = wire_to_reg_.find(key);
  if (it == wire_to_reg_.end()) {
    IRegister *nreg = new IRegister(tab_, reg->GetName());
    tab_->registers_.push_back(nreg);
    wire_to_reg_[key] = nreg;
    nreg->SetStateLocal(true);
    nreg->value_type_ = reg->value_type_;
    reg = nreg;
  } else {
    reg = it->second;
  }
  return reg;
}

void Pipeliner::UpdateCounterRead() {
  IRegister *counter = lb_->GetCounterRegister();
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
  IRegister *orig_counter = lb_->GetCounterRegister();
  for (int i = 0; i < llen; ++i) {
    IRegister *counter = new IRegister(tab_, RegName("ps", i));
    counter->value_type_ = orig_counter->value_type_;
    tab_->registers_.push_back(counter);
    ostream &os = opt_log_->Reg(counter);
    os << "counter:" << i;
    counters_.push_back(counter);
    IRegister *counter_wire = new IRegister(tab_, RegName("psw", i));
    counter_wire->SetStateLocal(true);
    counter_wire->value_type_ = orig_counter->value_type_;
    tab_->registers_.push_back(counter_wire);
    counter_wires_.push_back(counter_wire);
  }
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
      IInsn *insn = new IInsn(assign_);
      st->insns_.push_back(insn);
      insn->inputs_.push_back(counters_[j]);
      insn->outputs_.push_back(counters_[j + 1]);
    }
  }
}

void Pipeliner::SetupCounterIncrement() {
  // Assign at the prologue.
  // counter0 <- counter initial.
  IInsn *insn = new IInsn(DesignUtil::FindAssignResource(tab_));
  IRegister *counter_initial = lb_->GetInitialCounterValue();
  insn->inputs_.push_back(counter_initial);
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
  IRegister *one = DesignTool::AllocConstNum(tab_, cwidth, 1);
  for (int i = 0; i < llen; ++i) {
    IInsn *add_insn = new IInsn(adder);
    add_insn->inputs_.push_back(counter0);
    add_insn->inputs_.push_back(one);
    add_insn->outputs_.push_back(counter_wires_[i]);
    pipeline_stages_[i * interval_ + (interval_ - 1)]->insns_.push_back(
        add_insn);
    IInsn *wire_to_reg = new IInsn(assign_);
    wire_to_reg->inputs_.push_back(counter_wires_[i]);
    wire_to_reg->outputs_.push_back(counter0);
    pipeline_stages_[i * interval_ + (interval_ - 1)]->insns_.push_back(
        wire_to_reg);
  }
}

void Pipeliner::SetupExit() {
  int llen = ssch_->GetMacroStageCount();
  IState *st0 = pipeline_stages_[(llen - 1) * interval_];
  IState *st = pipeline_stages_[(llen - 1) * interval_ + (interval_ - 1)];
  IInsn *tr_insn = DesignUtil::GetTransitionInsn(st);
  // next, current
  tr_insn->target_states_.push_back(st0);
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
  IRegister *orig_counter = lb_->GetCounterRegister();
  return orig_counter->GetName() + base + Util::Itoa(index);
}

void Pipeliner::PrepareRegWriteReadPipelineStages() {
  for (auto p : reg_info_->GetWRDeps()) {
    WRDep *d = p.second;
    PipelineRegs(d->write_mst_index_, d->read_mst_index_, d->macro_stage_regs_);
  }
}

void Pipeliner::PipelineRegs(int start, int end, map<int, IRegister *> &regs) {
  vector<pair<int, int>> v =
      scheduled_shape_->GetPipeLineIndexRange(start, end);
  // 0:   0
  // 1:   0 1
  //      ...
  // mst: 0 1 ... mindex ...
  // *            r[mindex] <- r[mindex-1]
  //      ...
  for (auto &q : v) {
    int macrostage = q.first;
    int mindex = q.second;
    if (mindex == start) {
      // rewrite of insn->outputs_ is performed later.
      continue;
    }
    // Last local stage of the macro stage.
    int pindex = macrostage * interval_ + (interval_ - 1);
    IState *pst = pipeline_stages_[pindex];
    IInsn *insn = new IInsn(assign_);
    IRegister *src = regs[mindex - 1];
    insn->inputs_.push_back(src);
    IRegister *dst = regs[mindex];
    insn->outputs_.push_back(dst);
    pst->insns_.push_back(insn);
    ostream &os = opt_log_->Insn(insn);
    os << "~";
  }
}

void Pipeliner::PrepareRegWriteReadPipelineRegs() {
  for (auto p : reg_info_->GetWRDeps()) {
    WRDep *d = p.second;
    IRegister *reg = p.first;
    for (int i = d->write_mst_index_; i < d->read_mst_index_; ++i) {
      IRegister *r = new IRegister(tab_, reg->GetName() + "_s" + Util::Itoa(i));
      r->value_type_ = reg->value_type_;
      tab_->registers_.push_back(r);
      d->macro_stage_regs_[i] = r;
    }
  }
}

void Pipeliner::PrepareInsnCondRegPipelineRegs() {
  vector<IRegister *> cond_regs = insn_cond_->GetConditions();
  for (IRegister *reg : cond_regs) {
    auto &m = insn_cond_->GetConditionRegStages(reg);
    int cond_st = insn_cond_->GetConditionStateIndex(reg);
    int last_st = insn_cond_->GetConditionLastUseStateIndex(reg);
    for (int i = cond_st; i < last_st; ++i) {
      IRegister *r =
          new IRegister(tab_, reg->GetName() + "_cs" + Util::Itoa(i));
      r->value_type_.SetWidth(0);
      m[i] = r;
      tab_->registers_.push_back(r);
    }
  }
}

void Pipeliner::PrepareInsnCondRegPipelineStages() {
  vector<IRegister *> cond_regs = insn_cond_->GetConditions();
  for (IRegister *reg : cond_regs) {
    int cond_mst = insn_cond_->GetConditionStateIndex(reg);
    int last_mst = insn_cond_->GetConditionLastUseStateIndex(reg);
    auto &m = insn_cond_->GetConditionRegStages(reg);
    vector<pair<int, int>> v =
        scheduled_shape_->GetPipeLineIndexRange(cond_mst, last_mst);
    PipelineRegs(cond_mst, last_mst, m);
  }
}

void Pipeliner::SetInstructionConditions() {}

IRegister *Pipeliner::LookupStagedReg(int lidx, IRegister *reg) {
  WRDep *dep = reg_info_->GetWRDep(reg);
  if (dep == nullptr) {
    return reg;
  }
  auto jt = dep->macro_stage_regs_.find(lidx - 1);
  if (jt != dep->macro_stage_regs_.end()) {
    return jt->second;
  }
  return reg;
}

void Pipeliner::UpdateRegWriteReadPipelineAccess() {
  for (IState *st : pipeline_stages_) {
    vector<IInsn *> new_insns;
    for (IInsn *insn : st->insns_) {
      for (int i = 0; i < insn->outputs_.size(); ++i) {
        IRegister *reg = insn->outputs_[i];
        WRDep *dep = reg_info_->GetWRDep(reg);
        if (dep == nullptr) {
          continue;
        }
        // Initial written reg.
        IRegister *preg0 = (dep->macro_stage_regs_.begin()->second);
        if (reg->IsStateLocal()) {
          // preg0 <- reg
          IInsn *a = new IInsn(assign_);
          a->inputs_.push_back(reg);
          a->outputs_.push_back(preg0);
          new_insns.push_back(a);
        } else {
          IRegister *w = new IRegister(tab_, reg->GetName());
          w->SetStateLocal(true);
          w->value_type_ = reg->value_type_;
          tab_->registers_.push_back(w);
          insn->outputs_[i] = w;
          // reg <- w
          IInsn *a = new IInsn(assign_);
          a->inputs_.push_back(w);
          a->outputs_.push_back(reg);
          new_insns.push_back(a);
          // preg0 <- w
          IInsn *b = new IInsn(assign_);
          b->inputs_.push_back(w);
          b->outputs_.push_back(preg0);
          new_insns.push_back(b);
        }
      }
    }
    for (IInsn *insn : new_insns) {
      st->insns_.push_back(insn);
    }
  }
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
