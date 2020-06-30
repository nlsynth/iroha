#include "writer/verilog/ext_io_accessor.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/verilog/ext_io.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/wire_set.h"

namespace iroha {
namespace writer {
namespace verilog {

ExtIOAccessor::ExtIOAccessor(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void ExtIOAccessor::BuildResource() {
}

void ExtIOAccessor::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsExtInputAccessor(*klass)) {
    ostream &ws = tab_.InsnWireValueSectionStream();
    IResource *parent = res_.GetParentResource();
    string rn = ExtIO::InputRegName(*parent);
    ws << "  assign "
       << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = " << wire::Names::AccessorWire(rn, &res_, "r") << ";\n";
  }
}

void ExtIOAccessor::CollectNames(Names *names) {
}

bool ExtIOAccessor::UseOutput(const IResource *accessor) {
  bool o;
  bool p;
  OutputFeature(accessor, &o, &p);
  return o;
}

bool ExtIOAccessor::UsePeek(const IResource *accessor) {
  bool o;
  bool p;
  OutputFeature(accessor, &o, &p);
  return p;
}

void ExtIOAccessor::OutputFeature(const IResource *accessor, bool *o, bool *p) {
  *o = false;
  *p = false;
  auto *klass = accessor->GetClass();
  if (!resource::IsExtOutputAccessor(*klass)) {
    return;
  }
  vector<IInsn *> insns = DesignUtil::GetInsnsByResource(accessor);
  for (auto *insn : insns) {
    if (insn->inputs_.size() > 0) {
      *o = true;
    }
    if (insn->outputs_.size() > 0) {
      *p = true;
    }
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
