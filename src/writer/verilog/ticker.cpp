#include "writer/verilog/ticker.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"
#include "writer/verilog/module.h"
#include "writer/verilog/wire/accessor_info.h"
#include "writer/verilog/wire/wire_set.h"

namespace iroha {
namespace writer {
namespace verilog {

Ticker::Ticker(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void Ticker::BuildResource() {
  string n = TickerName();
  ostream &rs = tab_.ResourceSectionStream();
  rs << "  reg [31:0] " << n << ";\n";
  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << n << " <= 0;\n";
  ostream &ss = tab_.StateOutputSectionStream();
  ss << "      " << n << " <= " << n << " + 1";
  if (HasDecrement()) {
    ss << " - " << BuildDecrement();
  }
  ss << ";\n";
  BuildAccessorWire();
}

void Ticker::BuildInsn(IInsn *insn, State *st) {
  string n = TickerName();
  if (insn->outputs_.size() > 0) {
    ostream &ws = tab_.InsnWireValueSectionStream();
    ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = " << n << ";\n";
  } else {
    ostream &os = st->StateBodySectionStream();
    os << "          $display(\"ticker:%d\", " << n << ");\n";
  }
}

string Ticker::TickerName() {
  return TickerName(res_);
}

string Ticker::TickerName(const IResource &res) {
  return "ticker_" + Util::Itoa(res.GetTable()->GetId()) + "_" +
    Util::Itoa(res.GetId());
}

bool Ticker::HasDecrement() {
  map<IState *, IInsn *> callers;
  CollectResourceCallers("*", &callers);
  for (auto it : callers) {
    IInsn *insn = it.second;
    if (insn->inputs_.size() > 0) {
      return true;
    }
  }
  return false;
}

string Ticker::BuildDecrement() {
  map<IState *, IInsn *> allCallers;
  CollectResourceCallers("*", &allCallers);
  map<IState *, IInsn *> callers;
  for (auto it : allCallers) {
    IInsn *insn = it.second;
    if (insn->inputs_.size() > 0) {
      callers[it.first] = insn;
    }
  }
  return SelectValueByStateWithCallers(callers, "0");
}

void Ticker::BuildAccessorWire() {
  auto &conn = tab_.GetModule()->GetConnection();
  const vector<IResource *> &acs = conn.GetTickerAccessors(&res_);
  if (acs.size() == 0) {
    return;
  }
  string rn = TickerName();
  ostream &rvs = tab_.ResourceValueSectionStream();
  rvs << "  assign " << wire::Names::ResourceWire(rn, "c") << " = "
      << TickerName() << ";\n";
  wire::WireSet *ws = new wire::WireSet(*this, rn);
  for (auto *ac : acs) {
    wire::AccessorInfo *ainfo = ws->AddAccessor(ac);
    ainfo->SetDistance(ac->GetParams()->GetDistance());
    ainfo->AddSignal("c", wire::AccessorSignalType::ACCESSOR_READ_ARG, 32);
  }
  ws->Build();
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
