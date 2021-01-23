#include "writer/verilog/debug_resource.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/state.h"

static const char I[] = "          ";

namespace iroha {
namespace writer {
namespace verilog {

DebugResource::DebugResource(const IResource &res, const Table &table)
    : Resource(res, table) {}

void DebugResource::BuildResource() {}

void DebugResource::BuildInsn(IInsn *insn, State *st) {
  auto *rc = res_.GetClass();
  ostream &os = st->StateTransitionSectionStream();
  BeginCondition(insn, st->GetNames(), os);
  if (resource::IsPrint(*rc)) {
    Print(insn, st);
  } else if (resource::IsAssert(*rc)) {
    Assert(insn, st);
  }
  EndCondition(insn, os);
}

void DebugResource::CollectNames(Names *names) {}

void DebugResource::Print(IInsn *insn, State *st) {
  ostream &os = st->StateTransitionSectionStream();
  for (int i = 0; i < insn->inputs_.size(); ++i) {
    IRegister *reg = insn->inputs_[i];
    os << I << "$display(\"%m:" << insn->GetResource()->GetTable()->GetId()
       << ":" << insn->GetId() << " %d\", "
       << InsnWriter::RegisterValue(*reg, st->GetNames()) << ");\n";
  }
}

void DebugResource::Assert(IInsn *insn, State *st) {
  ostream &os = st->StateTransitionSectionStream();
  os << I << "if (!(";
  for (int i = 0; i < insn->inputs_.size(); ++i) {
    if (i > 0) {
      os << " && ";
    }
    IRegister *reg = insn->inputs_[i];
    os << "(" << InsnWriter::RegisterValue(*reg, st->GetNames()) << " === 1)";
  }
  os << ")) begin\n";
  os << I << "  $display(\"ASSERTION FAILURE: " << st->GetIState()->GetId()
     << "\");\n";
  os << I << "end\n";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
