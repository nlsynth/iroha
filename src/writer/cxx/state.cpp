#include "writer/cxx/state.h"

#include "iroha/i_design.h"
#include "writer/cxx/class_writer.h"
#include "writer/cxx/table.h"

namespace iroha {
namespace writer {
namespace cxx {

State::State(IState *st, Table *tab) : i_st_(st), tab_(tab) {
}

void State::Build() {
  ClassWriter *cw = tab_->GetClassWriter();
  cw->AddMethod(GetMethodName(), "void");
}

IState *State::GetIState() const {
  return i_st_;
}

string State::GetMethodName() {
  return "s_" + Util::Itoa(i_st_->GetId());
}

}  // namespace cxx
}  // namespace writer
}  // namespace iroha
