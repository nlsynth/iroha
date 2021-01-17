#include "writer/verilog/debug_resource.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/state.h"

namespace iroha {
namespace writer {
namespace verilog {

DebugResource::DebugResource(const IResource &res, const Table &table)
    : Resource(res, table) {}

void DebugResource::BuildResource() {}

void DebugResource::BuildInsn(IInsn *insn, State *st) {
  ostream &ts = st->StateTransitionSectionStream();
  InsnWriter writer(insn, st, ts);
  auto *rc = res_.GetClass();
  const string &rc_name = rc->GetName();
  if (rc_name == resource::kPrint) {
    writer.Print();
  } else if (rc_name == resource::kAssert) {
    writer.Assert();
  }
}

void DebugResource::CollectNames(Names *names) {}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
