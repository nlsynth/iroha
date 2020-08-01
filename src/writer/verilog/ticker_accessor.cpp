#include "writer/verilog/ticker_accessor.h"

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
}

void TickerAccessor::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  IResource *parent = res_.GetParentResource();
  ostream &ws = tab_.InsnWireValueSectionStream();
  if (resource::IsTickerAccessor(*klass)) {
    string rn = Ticker::TickerName(*parent);
    ws << "  assign "
       << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = " << wire::Names::AccessorWire(rn, &res_, "c") << ";\n";
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
