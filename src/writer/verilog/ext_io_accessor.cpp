#include "writer/verilog/ext_io_accessor.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
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
  auto *klass = res_.GetClass();
  if (resource::IsExtOutputAccessor(*klass)) {
    BuildOutputResource();
  }
}

void ExtIOAccessor::BuildOutputResource() {
  map<IState *, IInsn *> callers;
  CollectOutputCallers(&callers);
  string rn = GetName();
  ostream &rvs = tab_.ResourceValueSectionStream();
  rvs << "  assign " << wire::Names::AccessorWire(rn, &res_, "wen") << " = ";
  WriteStateUnion(callers, rvs);
  rvs << ";\n";
  rvs << "  assign " << wire::Names::AccessorWire(rn, &res_, "w") << " = ";
  string v;
  for (auto &p : callers) {
    if (v.empty()) {
      v = InsnWriter::InsnSpecificWireName(*(p.second));
    } else {
      v = "(" + tab_.GetStateCondition(p.first) + ") ? " +
	InsnWriter::InsnSpecificWireName(*(p.second)) + " : (" + v + ")";
    }
  }
  rvs << v << ";\n";
}

void ExtIOAccessor::CollectOutputCallers(map<IState *, IInsn *> *callers) {
  map<IState *, IInsn *> all_callers;
  CollectResourceCallers("", &all_callers);
  for (auto it : all_callers) {
    IInsn *insn = it.second;
    if (insn->inputs_.size() > 0) {
      (*callers)[it.first] = it.second;
    }
  }
}

void ExtIOAccessor::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  IResource *parent = res_.GetParentResource();
  ostream &ws = tab_.InsnWireValueSectionStream();
  if (resource::IsExtInputAccessor(*klass)) {
    string rn = ExtIO::InputRegName(*parent);
    ws << "  assign "
       << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = " << wire::Names::AccessorWire(rn, &res_, "r") << ";\n";
  }
  if (resource::IsExtOutputAccessor(*klass) &&
      insn->inputs_.size() > 0) {
    string rn = ExtIO::OutputRegName(*parent);
    int width = res_.GetParentResource()->GetParams()->GetWidth();
    ws << "  wire " << Table::WidthSpec(width)
       << InsnWriter::InsnSpecificWireName(*insn) << ";\n";
    ws << "  assign "
       << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = " << wire::Names::AccessorWire(rn, &res_, "w") << ";\n";
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

string ExtIOAccessor::GetName() {
  IResource *parent = res_.GetParentResource();
  auto *klass = parent->GetClass();
  if (resource::IsExtOutput(*klass)) {
    return ExtIO::OutputRegName(*parent);
  } else {
    return ExtIO::InputRegName(*parent);
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
