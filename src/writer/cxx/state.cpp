#include "writer/cxx/state.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/cxx/class_writer.h"
#include "writer/cxx/table.h"

#include <sstream>

using std::ostringstream;

namespace iroha {
namespace writer {
namespace cxx {

State::State(IState *st, Table *tab) : i_st_(st), tab_(tab) {
}

void State::Build() {
  ClassWriter *cw = tab_->GetClassWriter();
  ClassMember *cm = cw->AddMethod(GetMethodName(), "void");
  ostringstream os;
  for (auto *insn : i_st_->insns_) {
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

void State::WriteInsn(IInsn *insn, ostream &os) {
  IResource *res = insn->GetResource();
  IResourceClass *rc = res->GetClass();
  if (resource::IsTransition(*rc)) {
    WriteTransitionInsn(insn, os);
  }
}

void State::WriteTransitionInsn(IInsn *insn, ostream &os) {
  if (insn->target_states_.size() == 1) {
    os << "    " << tab_->GetStateVariableName() << " = "
       << insn->target_states_[0]->GetId() << ";\n";
  } else if (insn->target_states_.size() == 2) {
    os << "    if (!" << RegValue(insn->inputs_[0]) << ") {\n";
    os << "      " << tab_->GetStateVariableName() << " = "
       << insn->target_states_[0]->GetId() << ";\n";
    os << "    } else {\n";
    os << "      " << tab_->GetStateVariableName() << " = "
       << insn->target_states_[1]->GetId() << ";\n";
    os << "    }\n";
  }
}

string State::RegValue(IRegister *reg) {
  return reg->GetName();
}

}  // namespace cxx
}  // namespace writer
}  // namespace iroha
