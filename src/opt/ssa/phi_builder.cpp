#include "opt/ssa/phi_builder.h"

#include "design/design_tool.h"
#include "iroha/resource_class.h"
#include "iroha/stl_util.h"
#include "opt/bb_set.h"
#include "opt/data_flow.h"


namespace iroha {
namespace opt {
namespace ssa {

PhiBuilder::PhiBuilder(ITable *tab, DebugAnnotation *annotation)
  : table_(tab), annotation_(annotation), phi_(nullptr) {
}

PhiBuilder::~PhiBuilder() {
  STLDeleteValues(&phis_);
  STLDeleteSecondElements(&reg_info_);
}

void PhiBuilder::Perform() {
  bset_.reset(BBSet::Create(table_, annotation_));
  data_flow_.reset(DataFlow::Create(bset_.get(), annotation_));
  phi_ = DesignTool::GetResource(table_, resource::kPhi);

  for (BB *bb : bset_->bbs_) {
    CalculatePHIInputsForBB(bb);
  }

  for (PHI *phi : phis_) {
    UpdatePHIInputs(phi);
  }

  for (RegDef *reg_def : data_flow_->all_defs_) {
    reg_def_map_[reg_def->insn].insert(reg_def);
  }

  for (BB *bb : bset_->bbs_) {
    UpdateVersionsForBB(bb);
  }
}

void PhiBuilder::CalculatePHIInputsForBB(BB *bb) {
  set<IInsn *> phi_insns;
  for (IInsn *insn : bb->states_[0]->insns_) {
    if (insn->GetResource() == phi_) {
      phi_insns.insert(insn);
    }
  }
  vector<RegDef *> reaches;
  data_flow_->GetReachDefs(bb, &reaches);
  map<IRegister *, set<RegDef *> > per_reg_defs;
  for (RegDef *reg_def : reaches) {
    per_reg_defs[reg_def->reg].insert(reg_def);
  }
  for (IInsn *phi_insn : phi_insns) {
    IRegister *reg = *(phi_insn->outputs_.begin());
    set<RegDef *> &reaching = per_reg_defs[reg];
    PHI *phi = new PHI;
    phi->insn_ = phi_insn;
    phi->defs_ = reaching;
    phis_.push_back(phi);
  }
}

void PhiBuilder::UpdatePHIInputs(PHI *phi) {
  for (RegDef *reg_def : phi->defs_) {
    IRegister *reg = FindVersionedReg(reg_def);
    phi->insn_->inputs_.push_back(reg);
  }
}

void PhiBuilder::UpdateVersionsForBB(BB *bb) {
  vector<RegDef *> reaches;
  data_flow_->GetReachDefs(bb, &reaches);
  map<IRegister *, RegDef *> last_defs;
  for (RegDef *reg_def : reaches) {
    last_defs[reg_def->reg] = reg_def;
  }
  for (IState *st : bb->states_) {
    for (IInsn *insn : st->insns_) {
      UpdateVersionsForInsn(&last_defs, insn);
    }
  }
}

void PhiBuilder::UpdateVersionsForInsn(map<IRegister *, RegDef *> *last_defs,
				       IInsn *insn) {
  if (insn->GetResource() != phi_) {
    for (auto it = insn->inputs_.begin(); it != insn->inputs_.end(); ++it) {
      IRegister *ireg = *it;
      if (last_defs->find(ireg) != last_defs->end()) {
	RegDef *reg_def = (*last_defs)[ireg];
	*it = FindVersionedReg(reg_def);
      }
    }
  }
  for (auto it = insn->outputs_.begin(); it != insn->outputs_.end(); ++it) {
    set<RegDef *> &defs = reg_def_map_[insn];
    RegDef *reg_def = nullptr;
    for (RegDef *d : defs) {
      if (d->reg == *it) {
	reg_def = d;
      }
    }
    (*last_defs)[reg_def->reg] = reg_def;
    *it = FindVersionedReg(reg_def);
  }
}

int PhiBuilder::GetVersionFromDefInfo(RegDef *reg_def) {
  auto it = def_versions_.find(reg_def);
  if (it == def_versions_.end()) {
    int v = def_versions_.size() + 1;
    def_versions_[reg_def] = v;
    return v;
  }
  return it->second;
}

IRegister *PhiBuilder::FindVersionedReg(RegDef *reg_def) {
  PerRegister *pr = reg_info_[reg_def->reg];
  if (pr == nullptr) {
    pr = new PerRegister;
    reg_info_[reg_def->reg] = pr;
  }
  int version = GetVersionFromDefInfo(reg_def);
  IRegister *reg = pr->versions_[version];
  if (reg == nullptr) {
    reg = new IRegister(table_, reg_def->reg->GetName() + "_v" +
			Util::Itoa(version));
    pr->versions_[version] = reg;
  }
  return reg;
}

}  // namespace ssa
}  // namespace opt
}  // namespace iroha
