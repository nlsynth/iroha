#include "writer/verilog/ticker_accessor.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/table.h"
#include "writer/verilog/ticker.h"
#include "writer/verilog/wire/wire_set.h"

namespace iroha {
namespace writer {
namespace verilog {

TickerAccessor::TickerAccessor(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void TickerAccessor::BuildResource() {
  map<IState *, IInsn *> callers;
  CollectDecrementCallers(&callers);
  if (callers.size() == 0) {
    return;
  }
  IResource *parent = res_.GetParentResource();
  string rn = Ticker::TickerName(*parent);
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

void TickerAccessor::BuildInsn(IInsn *insn, State *st) {
  IResource *parent = res_.GetParentResource();
  ostream &ws = tab_.InsnWireValueSectionStream();
  if (insn->outputs_.size() == 1) {
    // get
    string rn = Ticker::TickerName(*parent);
    ws << "  assign "
       << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = " << wire::Names::AccessorWire(rn, &res_, "c") << ";\n";
  }
  if (insn->inputs_.size() == 1) {
    // decrement
    string rn = Ticker::TickerName(*parent);
    ws << "  wire [31:0] "
       << InsnWriter::InsnSpecificWireName(*insn) << ";\n";
    ws << "  assign "
       << InsnWriter::InsnSpecificWireName(*insn)
       << " = "
       << InsnWriter::RegisterValue(*insn->inputs_[0], tab_.GetNames())
       << ";\n";
  }
}

bool TickerAccessor::UseDecrement(const IResource *accessor) {
  auto *klass = accessor->GetClass();
  if (!resource::IsTickerAccessor(*klass)) {
    return false;
  }
  vector<IInsn *> insns = DesignUtil::GetInsnsByResource(accessor);
  for (auto *insn : insns) {
    if (insn->inputs_.size() > 0) {
      return true;
    }
  }
  return false;
}

void TickerAccessor::CollectDecrementCallers(map<IState *, IInsn *> *callers) {
  map<IState *, IInsn *> all_callers;
  CollectResourceCallers("", &all_callers);
  for (auto it : all_callers) {
    IInsn *insn = it.second;
    if (insn->inputs_.size() > 0) {
      (*callers)[it.first] = it.second;
    }
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
