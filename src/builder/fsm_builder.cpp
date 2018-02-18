#include "builder/fsm_builder.h"

#include "builder/exp_builder.h"
#include "builder/reader.h"
#include "iroha/i_design.h"

namespace iroha {
namespace builder {

FsmBuilder::FsmBuilder(ITable *table, ExpBuilder *builder)
  : table_(table), builder_(builder), initial_state_id_(-1) {
  Init();
}

void FsmBuilder::Init() {
  for (auto *res : table_->resources_) {
    resources_[res->GetId()] = res;
  }
  for (auto *reg : table_->registers_) {
    registers_[reg->GetId()] = reg;
  }
}

void FsmBuilder::AddState(Exp *e) {
  if (e->vec.size() < 2) {
    builder_->SetError() << "Invalid state definition";
    return;
  }
  IState *st = new IState(table_);
  int st_id = Util::Atoi(e->vec[1]->atom.str);
  st->SetId(st_id);
  table_->states_.push_back(st);
  if (states_[st_id] != nullptr) {
    builder_->SetError() << "Duplicated state id: " << st_id;
  }
  states_[st_id] = st;
  exps_[st_id] = e;
}

void FsmBuilder::ResolveInsns() {
  for (auto p : exps_) {
    Exp *e = p.second;
    int st_id = p.first;
    IState *st = states_[st_id];
    if (!st) {
      // Assuming the error was already reported.
      continue;
    }

    map<Exp *, IInsn *> exp_to_insn;
    for (int i = 2; i < e->vec.size(); ++i) {
      const string &tag = e->vec[i]->vec[0]->atom.str;
      if (tag == "INSN") {
	IInsn *insn = BuildInsn(e->vec[i]);
	exp_to_insn[e->vec[i]] = insn;
	if (insn != nullptr) {
	  st->insns_.push_back(insn);
	}
      } else {
	builder_->SetError() << "Only INSN is allowed in a state";
      }
    }
    // Post process to resolve depending insns.
    for (int i = 2; i < e->vec.size(); ++i) {
      const string &tag = e->vec[i]->vec[0]->atom.str;
      if (tag == "INSN") {
	ResolveDependingInsns(e->vec[i], exp_to_insn[e->vec[i]]);
      }
    }
  }

  if (initial_state_id_ > -1) {
    IState *initial_st = states_[initial_state_id_];
    if (initial_st == nullptr) {
      builder_->SetError() << "Unknown initial state id: "
			   << initial_state_id_;
      return;
    }
    table_->SetInitialState(initial_st);
  }
}

void FsmBuilder::ResolveDependingInsns(Exp *e, IInsn *insn) {
  if (e->vec.size() < 9) {
    return;
  }
  for (auto *dep : e->vec[8]->vec) {
    int insn_id = Util::Atoi(dep->atom.str);
    insn->depending_insns_.push_back(insns_[insn_id]);
  }
}

void FsmBuilder::SetInitialState(Exp *e) {
  initial_state_id_ = Util::Atoi(e->vec[1]->atom.str);
}

IInsn *FsmBuilder::BuildInsn(Exp *e) {
  // 0: INSN
  // 1: id
  // 2: resource name
  // 3: resource id
  // 4: operand
  // 5: transition targets
  // 6: inputs
  // 7: outputs
  // 8: depending insns (optional)
  if (e->vec.size() < 8) {
    builder_->SetError() << "Malformed INSN";
    return nullptr;
  }
  int res_id = Util::Atoi(e->vec[3]->atom.str);
  IResource *res = resources_[res_id];
  if (!res) {
    builder_->SetError() << "Unknown resource id: " << res_id;
    return nullptr;
  }
  IInsn *insn = new IInsn(res);
  int id = Util::Atoi(e->vec[1]->atom.str);
  insn->SetId(id);
  insns_[id] = insn;
  if (e->vec[4]->vec.size() > 0) {
    insn->SetOperand(e->vec[4]->vec[0]->atom.str);
  }
  for (auto *tr : e->vec[5]->vec) {
    int st_id = Util::Atoi(tr->atom.str);
    IState *st = states_[st_id];
    if (!st) {
      builder_->SetError() << "Unknown state id: " << st_id;
      return nullptr;
    }
    insn->target_states_.push_back(st);
  }
  for (auto *input : e->vec[6]->vec) {
    BuildInsnParams(input, &insn->inputs_);
  }
  for (auto *output : e->vec[7]->vec) {
    BuildInsnParams(output, &insn->outputs_);
  }
  return insn;
}

void FsmBuilder::BuildInsnParams(Exp *e, vector<IRegister *> *regs) {
  int reg_id = Util::Atoi(e->atom.str);
  IRegister *reg = registers_[reg_id];
  if (!reg) {
    builder_->SetError() << "Unknown register id: " << reg_id;
    return;
  }
  regs->push_back(reg);
}

}  // namespace builder
}  // namespace iroha
