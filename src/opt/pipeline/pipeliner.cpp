#include "opt/pipeline/pipeliner.h"

#include "design/design_tool.h"
#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/stl_util.h"
#include "opt/loop/loop_block.h"
#include "opt/optimizer_log.h"
#include "opt/pipeline/shape.h"
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

Pipeliner::~Pipeliner() { STLDeleteSecondElements(&wr_deps_); }

bool Pipeliner::Pipeline() {
  lb_->Annotate(opt_log_);
  ostream &os = opt_log_->GetDumpStream();
  os << "Pipeliner " << ssch_->GetMacroStageCount() << " states, "
     << lb_->GetLoopCount() << " loop, interval=" << interval_ << " <br/>\n";
  if (!CollectWRRegs()) {
    os << "Give up due to multiple writes<br/>\n";
    return false;
  }
  // prepare states.
  prologue_st_ = new IState(tab_);
  tab_->states_.push_back(prologue_st_);
  int plen = ssch_->GetPipelineStageLength();
  for (int i = 0; i < plen; ++i) {
    IState *st = new IState(tab_);
    pipeline_stages_.push_back(st);
    tab_->states_.push_back(st);
  }
  PrepareRegPipeline();
  Shape shape(ssch_);
  vector<pair<int, int>> loc = shape.GetPipelineLocation();
  for (pair<int, int> &p : loc) {
    PlaceState(p.first, p.second);
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
    UpdateRegs(pst, lidx, false, insn->inputs_, &new_insn->inputs_);
    UpdateRegs(pst, lidx, true, insn->outputs_, &new_insn->outputs_);
    pst->insns_.push_back(new_insn);
    insn_to_stage_[new_insn] = lidx;
  }
}

void Pipeliner::UpdateRegs(IState *st, int lidx, bool is_output,
                           vector<IRegister *> &src, vector<IRegister *> *dst) {
  for (IRegister *reg : src) {
    if (reg->IsStateLocal()) {
      reg = MayUpdateWireReg(st, reg);
    } else if (reg->IsNormal() && !is_output) {
      reg = LookupStagedReg(lidx, reg);
    }
    dst->push_back(reg);
  }
}

IRegister *Pipeliner::MayUpdateWireReg(IState *st, IRegister *reg) {
  auto key = make_tuple(st, reg);
  auto it = wire_to_reg_.find(key);
  if (it == wire_to_reg_.end()) {
    IRegister *nreg = new IRegister(tab_, reg->GetName());
    tab_->registers_.push_back(nreg);
    wire_to_reg_[key] = nreg;
    nreg->SetStateLocal(true);
    nreg->value_type_ = reg->value_type_;
    reg = nreg;
  }
  return reg;
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

bool Pipeliner::CollectWRRegs() {
  map<IRegister *, int> write_pos;
  map<IRegister *, int> last_read_pos;
  int sindex = 0;
  for (IState *st : lb_->GetStates()) {
    for (IInsn *insn : st->insns_) {
      // Reads.
      for (IRegister *reg : insn->inputs_) {
        last_read_pos[reg] = sindex;
      }
      // Writes.
      for (IRegister *reg : insn->outputs_) {
        if (write_pos.find(reg) != write_pos.end()) {
          // write conflicts.
          return false;
        }
        if (!reg->IsNormal()) {
          continue;
        }
        write_pos[reg] = sindex;
      }
    }
    ++sindex;
  }
  ostream &os = opt_log_->GetDumpStream();
  for (auto it : last_read_pos) {
    IRegister *reg = it.first;
    auto jt = write_pos.find(reg);
    if (jt == write_pos.end()) {
      // no write in this loop.
      continue;
    }
    int windex = jt->second;
    int rindex = it.second;
    if (windex < rindex) {
      // Write -> Read.
      WRDep *dep = new WRDep();
      dep->wst_index_ = windex;
      dep->rst_index_ = rindex;
      wr_deps_[reg] = dep;
    }
  }
  if (wr_deps_.size() > 0) {
    os << "In pipleine register W-R dependencies.<br/>\n";
    for (auto p : wr_deps_) {
      WRDep *d = p.second;
      IRegister *reg = p.first;
      auto &sts = lb_->GetStates();
      os << "r_" << reg->GetId() << " " << reg->GetName()
         << " w:" << sts[d->wst_index_]->GetId()
         << " r:" << sts[d->rst_index_]->GetId() << "<br/>\n";
    }
  }
  return true;
}

void Pipeliner::PrepareRegPipeline() {
  Shape shape(ssch_);

  IResource *assign = DesignUtil::FindAssignResource(tab_);
  for (auto p : wr_deps_) {
    WRDep *d = p.second;
    IRegister *src = p.first;
    // WIP: Rewrite reads. May rewrite write to reg insn.
    vector<pair<int, int>> v =
        shape.GetPipeLineIndexRange(d->wst_index_, d->rst_index_);
    vector<IRegister *> regs;
    for (int i = d->wst_index_; i <= d->rst_index_; ++i) {
      IRegister *r = new IRegister(tab_, src->GetName() + "_s" + Util::Itoa(i));
      r->value_type_ = src->value_type_;
      regs.push_back(r);
      tab_->registers_.push_back(r);
      d->regs_[i] = r;
    }
    for (auto &p : v) {
      int macrostage = p.first;
      int lindex = p.second;
      int pindex = macrostage * interval_ + (interval_ - 1);
      IState *pst = pipeline_stages_[pindex];
      IInsn *insn = new IInsn(assign);
      insn->inputs_.push_back(src);
      IRegister *dst = regs[lindex - d->wst_index_];
      insn->outputs_.push_back(dst);
      pst->insns_.push_back(insn);
      ostream &os = opt_log_->Insn(insn);
      os << "~";
      // for next macro stage.
      src = dst;
    }
  }
}

IRegister *Pipeliner::LookupStagedReg(int lidx, IRegister *reg) {
  auto it = wr_deps_.find(reg);
  if (it == wr_deps_.end()) {
    return reg;
  }
  WRDep *dep = it->second;
  auto jt = dep->regs_.find(lidx);
  if (jt != dep->regs_.end()) {
    return jt->second;
  }
  return reg;
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
