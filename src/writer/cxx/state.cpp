#include "writer/cxx/state.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/cxx/class_writer.h"
#include "writer/cxx/resource.h"
#include "writer/cxx/table.h"

#include <map>
#include <set>
#include <sstream>

using std::map;
using std::ostringstream;
using std::set;

namespace iroha {
namespace writer {
namespace cxx {

class InsnDependencyResolver {
public:
  InsnDependencyResolver(vector<IInsn *> &insns) : insns_(insns) {
  }

  void Sort(vector<IInsn *> *insns) {
    for (auto *insn : insns_) {
      for (auto *oreg : insn->outputs_) {
	if (oreg->IsStateLocal()) {
	  wire_source_[oreg] = insn;
	}
      }
    }
    for (auto *insn : insns_) {
      ResolveInsnInputs(insn, insns);
    }
  }

private:
  void ResolveInsnInputs(IInsn *insn, vector<IInsn *> *result) {
    if (emitted_.find(insn) != emitted_.end()) {
      return;
    }
    for (auto *ireg : insn->inputs_) {
      if (!ireg->IsStateLocal() || ireg->IsConst()) {
	continue;
      }
      ResolveInsnInputs(wire_source_[ireg], result);
    }
    result->push_back(insn);
    emitted_.insert(insn);
  }

  map<IRegister *, IInsn *> wire_source_;
  set<IInsn *> emitted_;
  vector<IInsn *> &insns_;
};

State::State(IState *st, Table *tab) : i_st_(st), tab_(tab) {
}

void State::Build() {
  ClassWriter *cw = tab_->GetClassWriter();
  ClassMember *cm = cw->AddMethod(GetMethodName(), "void");
  ostringstream os;
  vector<IInsn *> insns;
  OrderInsns(&insns);
  for (auto *insn : insns) {
    WriteInsn(insn, os);
  }
  cm->body_ = os.str();
}

IState *State::GetIState() const {
  return i_st_;
}

string State::GetMethodName() {
  return "s_" + Util::Itoa(i_st_->GetId());
}

void State::OrderInsns(vector<IInsn *> *insns) {
  InsnDependencyResolver resolver(i_st_->insns_);
  resolver.Sort(insns);
}

void State::WriteInsn(IInsn *insn, ostream &os) {
  IResource *res = insn->GetResource();
  IResourceClass *rc = res->GetClass();
  if (resource::IsTransition(*rc)) {
    WriteTransitionInsn(insn, os);
  }
  Resource::WriteInsn(insn, os);
}

void State::WriteTransitionInsn(IInsn *insn, ostream &os) {
  if (insn->target_states_.size() == 1) {
    os << "    " << tab_->GetStateVariableName() << " = "
       << insn->target_states_[0]->GetId() << ";\n";
  } else if (insn->target_states_.size() == 2) {
    os << "    if (!" << Resource::RegValue(insn->inputs_[0]) << ") {\n";
    os << "      " << tab_->GetStateVariableName() << " = "
       << insn->target_states_[0]->GetId() << ";\n";
    os << "    } else {\n";
    os << "      " << tab_->GetStateVariableName() << " = "
       << insn->target_states_[1]->GetId() << ";\n";
    os << "    }\n";
  }
}

}  // namespace cxx
}  // namespace writer
}  // namespace iroha
