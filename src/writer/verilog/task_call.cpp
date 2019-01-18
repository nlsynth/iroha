#include "writer/verilog/task_call.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/state.h"
#include "writer/verilog/task.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

TaskCall::TaskCall(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void TaskCall::BuildResource() {
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);
  ostream &ss = tab_.StateOutputSectionStream();
  string en = Task::TaskEnablePin(*(res_.GetCalleeTable()), &res_);
  ostream &rs = tab_.ResourceSectionStream();
  rs << "  reg " << en << ";\n";
  rs << "  assign " << en << "_wire = " << en << ";\n";
  for (int i = 0; i < res_.input_types_.size(); ++i) {
    auto &type = res_.input_types_[i];
    string a = Task::TaskArgPin(*(res_.GetCalleeTable()), i,
				&res_);
    rs << "  reg " << Table::ValueWidthSpec(type) << " " << a << ";\n";
    rs << "  assign " << a << "_wire = " << a << ";\n";
  }
  ss << "      " << en << " <= " << JoinStatesWithSubState(callers, 0)
     << ";\n";
  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << en << " <= 0;\n";
}

void TaskCall::BuildInsn(IInsn *insn, State *st) {
  ostream &os = st->StateBodySectionStream();
  static const char I[] = "          ";
  string st_name = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  string ack =
    Task::TaskAckPin(*(res_.GetCalleeTable()), &res_) + "_wire";
  os << I << "if (" << st_name << " == 0) begin\n";
  ITable *callee = res_.GetCalleeTable();
  CHECK(res_.input_types_.size() == insn->inputs_.size());
  for (int i = 0; i < res_.input_types_.size(); ++i) {
    os << I << "  " << Task::TaskArgPin(*callee, i, &res_)
       << " <= " << InsnWriter::RegisterValue(*insn->inputs_[i],
					      tab_.GetNames())
       << ";\n";
  }
  os << I << "  if (" << ack << ") begin\n"
     << I << "    " << st_name << " <= 3;\n"
     << I << "  end else begin\n"
     << I << "  end\n"
     << I << "end\n";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
