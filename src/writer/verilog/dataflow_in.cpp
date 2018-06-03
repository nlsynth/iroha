#include "writer/verilog/dataflow_in.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "writer/module_template.h"
#include "writer/verilog/dataflow_table.h"
#include "writer/verilog/fifo.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/shared_reg.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

DataFlowIn::DataFlowIn(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void DataFlowIn::BuildResource() {
  IResource *parent = res_.GetParentResource();
  if (parent != nullptr &&
      resource::IsFifo(*(parent->GetClass()))) {
    ostream &rs = tab_.ResourceSectionStream();
    rs << "  assign " << Fifo::RReq(*(res_.GetParentResource()), &res_) << " = ";
    const DataFlowTable *df = tab_.GetDataFlowTable();
    if (df->CanBlock()) {
      rs << "!" << df->BlockingCondition() << ";\n";
    } else {
      rs << "1;\n";
    }
  }
}

void DataFlowIn::BuildInsn(IInsn *insn, State *st) {
  IResource *res = insn->GetResource();
  IResource *parent = res->GetParentResource();
  CHECK(parent != nullptr);
  auto &rc = *(parent->GetClass());
  if (parent != nullptr &&
      (resource::IsSharedReg(rc) ||
       resource::IsFifo(rc))) {
    int s = 0;
    for (int i = 0; i < insn->outputs_.size(); ++i) {
      ostream &ws = tab_.InsnWireValueSectionStream();
      ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, i)
	 << " = ";
      if (resource::IsFifo(rc)) {
	ws << Fifo::RData(*parent);
      } else {
	ws << SharedReg::RegName(*parent);
      }
      if (insn->outputs_.size() > 1) {
	IRegister *reg = insn->outputs_[i];
	int w = reg->value_type_.GetWidth();
	if (w == 0) {
	  w = 1;
	}
	ws << "[" << (s + w - 1) << ":" << s << "]";
	s += w;
      }
      ws << ";\n";
    }
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
