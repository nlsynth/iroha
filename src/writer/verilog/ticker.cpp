#include "writer/verilog/ticker.h"

#include "iroha/i_design.h"
#include "writer/module_template.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"
#include "writer/verilog/module.h"

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
  ss << "      " << n << " <= " << n << " + 1;\n";
}

void Ticker::BuildInsn(IInsn *insn, State *st) {
  ostream &os = st->StateBodySectionStream();
  string n = TickerName();
  os << "          $display(\"ticker:%d\", " << n << ");\n";
}

string Ticker::TickerName() {
  return "ticker_" + Util::Itoa(res_.GetTable()->GetId()) + "_" +
    Util::Itoa(res_.GetId());
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
