#include "writer/verilog/task.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/wire_set.h"

namespace iroha {
namespace writer {
namespace verilog {

const int Task::kTaskEntryStateId = -1;

Task::Task(const IResource &res, const Table &table)
  : Resource(res, table) {
}

bool Task::IsTask(const Table &table) {
  ITable *i_table = table.GetITable();
  IInsn *task_entry_insn = DesignUtil::FindTaskEntryInsn(i_table);
  if (task_entry_insn != nullptr) {
    auto rc = *(task_entry_insn->GetResource()->GetClass());
    return resource::IsTask(rc);
  }
  return false;
}

string Task::TaskEnableWire(const ITable &tab) {
  return TaskEnablePin(tab, nullptr) + "_wire";
}

string Task::TaskEnablePin(const ITable &tab, const IResource *caller) {
  return TaskPinPrefix(tab, caller) + "_en";
}

string Task::TaskAckPin(const ITable &tab, const IResource *caller) {
  return TaskPinPrefix(tab, caller) + "_ack";
}

string Task::TaskArgPin(const ITable &tab, int nth,
			const IResource *caller) {
  return TaskPinPrefix(tab, caller) + "_arg_" + Util::Itoa(nth);
}

string Task::TaskPinPrefix(const ITable &tab, const IResource *caller) {
  string s = "task_" + Util::Itoa(tab.GetModule()->GetId()) + "_" + Util::Itoa(tab.GetId());
  if (caller == nullptr) {
    return s;
  } else {
    return wire::AccessorInfo::AccessorName(s, caller);
  }
}

void Task::BuildResource() {
  auto *klass = res_.GetClass();
  CHECK(resource::IsTask(*klass));
  auto &conn = tab_.GetModule()->GetConnection();
  const auto &callers = conn.GetTaskCallers(&res_);
  if (callers.size() == 0) {
    return;
  }
  BuildWireSet();

  ostream &rs = tab_.ResourceSectionStream();
  // ACK
  string common_en = TaskEnablePin(*(tab_.GetITable()), nullptr) + "_wire";
  string ack = TaskAckPin(*(tab_.GetITable()), nullptr) + "_src";
  string ack_cond = ack + "_cond";
  rs << "  wire " << ack_cond << ";\n"
     << "  assign " << ack_cond << " = ("
     << tab_.StateVariable() << " == `"
     << tab_.StateName(Task::kTaskEntryStateId) << ") && " << common_en
     << ";\n";
  rs << "  assign " << TaskAckPin(*(tab_.GetITable()), nullptr) << "_wire" << " = " << ack << ";\n";
  ostream &fs = tab_.StateOutputSectionStream();
  fs << "      " << ack
     << " <= " << ack_cond
     << ";\n";
  rs << "  reg " << ack << ";\n";
  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << ack << " <= 0;\n";
  // Args
  for (int i = 0; i < res_.output_types_.size(); ++i) {
    auto &type = res_.output_types_[i];
    string a = TaskArgPin(*(tab_.GetITable()), i, nullptr) + "_reg";
    rs << "  reg " << Table::ValueWidthSpec(type) << " " << a << ";\n";
    is << "      " << a << " <= 0;\n";
    IInsn *task_entry_insn = DesignUtil::FindTaskEntryInsn(tab_.GetITable());
    rs << "  assign " << InsnWriter::InsnOutputWireName(*task_entry_insn, i)
       << " = " << a << ";\n";
  }
  // Capturing args
  bool is_first = true;
  for (IResource *caller : callers) {
    string e;
    if (!is_first) {
      e = "else ";
    }
    string en = TaskEnablePin(*(tab_.GetITable()), caller);
    fs << "      " << e << "if (" << ack_cond << " && " << en << "_wire) begin\n"
       << "      // Capturing args\n";
    for (int i = 0; i < res_.output_types_.size(); ++i) {
      string ad = TaskArgPin(*(tab_.GetITable()), i, nullptr);
      string as = TaskArgPin(*(tab_.GetITable()), i, caller);
      fs << "        " << ad << "_reg" << " <= " << as << "_wire;\n";
    }
    fs << "      end\n";
    is_first = false;
  }
}

void Task::BuildWireSet() {
  auto &conn = tab_.GetModule()->GetConnection();
  const auto &callers = conn.GetTaskCallers(&res_);
  wire::WireSet ws(*this, TaskPinPrefix(*(tab_.GetITable()), nullptr));
  for (auto *accessor : callers) {
    wire::AccessorInfo *ainfo = ws.AddAccessor(accessor);
    ainfo->AddSignal("en", wire::AccessorSignalType::ACCESSOR_REQ, 0);
    ainfo->AddSignal("ack", wire::AccessorSignalType::ACCESSOR_ACK, 0);
    for (int i = 0; i < res_.output_types_.size(); ++i) {
      auto &type = res_.output_types_[i];
      ainfo->AddSignal(TaskArgName(i),
		       wire::AccessorSignalType::ACCESSOR_WRITE_ARG,
		       type.GetWidth());
    }
  }
  ws.Build();
}

string Task::TaskArgName(int nth) {
  return "arg_" + Util::Itoa(nth);
}

void Task::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  CHECK(resource::IsTask(*klass));
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
