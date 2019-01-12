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
#include "writer/verilog/wire/inter_module_wire.h"
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

string Task::TaskEnablePin(const ITable &tab, const ITable *caller) {
  return TaskPinPrefix(tab, caller) + "_en";
}

string Task::TaskAckPin(const ITable &tab, const ITable *caller) {
  return TaskPinPrefix(tab, caller) + "_ack";
}

string Task::TaskArgPin(const ITable &tab, int nth, bool output,
			const ITable *caller) {
  string dir = output ? "o" : "i";
  return TaskPinPrefix(tab, caller) + "_arg_" + dir + Util::Itoa(nth);
}

string Task::TaskPinPrefix(const ITable &tab, const ITable *caller) {
  string s = "task_" + Util::Itoa(tab.GetModule()->GetId()) + "_" + Util::Itoa(tab.GetId());
  if (caller != nullptr) {
    IModule *caller_mod = caller->GetModule();
    s += "_" + Util::Itoa(caller_mod->GetId()) +
      "_" + Util::Itoa(caller->GetId());
  }
  return s;
}

void Task::BuildResource() {
  auto *klass = res_.GetClass();
  CHECK(resource::IsTask(*klass));

  auto &conn = tab_.GetModule()->GetConnection();
  const auto &callers = conn.GetTaskCallers(&res_);
  if (callers.size() == 0) {
    return;
  }
  // Inter module wires.
  wire::InterModuleWire wire(*this);
  for (IResource *caller : callers) {
    string en = TaskEnablePin(*(tab_.GetITable()), caller->GetTable());
    wire.AddWire(*caller, en, 0, false, true);
    string ack = TaskAckPin(*(tab_.GetITable()), caller->GetTable());
    wire.AddWire(*caller, ack, 0, true, false);
    for (int i = 0; i < res_.output_types_.size(); ++i) {
      auto &type = res_.output_types_[i];
      string a = TaskArgPin(*(tab_.GetITable()), i, false, caller->GetTable());
      wire.AddWire(*caller, a, type.GetWidth(), false, true);
    }
  }
  // EN
  vector<string> task_en;
  for (IResource *caller : callers) {
    task_en.push_back(TaskEnablePin(*(tab_.GetITable()), caller->GetTable()));
  }
  string common_en = TaskEnablePin(*(tab_.GetITable()), nullptr);
  ostream &rs = tab_.ResourceSectionStream();
  rs << "  wire " << common_en << ";\n";
  rs << "  assign " << common_en << " = " << Util::Join(task_en, " | ")
     << ";\n";
  // ACK
  string ack = TaskAckPin(*(tab_.GetITable()), nullptr);
  string ack_cond = ack + "_cond";
  rs << "  wire " << ack_cond << ";\n"
     << "  assign " << ack_cond << " = ("
     << tab_.StateVariable() << " == `"
     << tab_.StateName(Task::kTaskEntryStateId) << ") && " << common_en
     << ";\n";
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
    string a = TaskArgPin(*(tab_.GetITable()), i, false, nullptr);
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
    string en = TaskEnablePin(*(tab_.GetITable()), caller->GetTable());
    fs << "      " << e << "if (" << ack_cond << " && " << en << ") begin\n"
       << "      // Capturing args\n";
    for (int i = 0; i < res_.output_types_.size(); ++i) {
      string ad = TaskArgPin(*(tab_.GetITable()), i, false, nullptr);
      string as = TaskArgPin(*(tab_.GetITable()), i, false, caller->GetTable());
      fs << "        " << ad << " <= " << as << ";\n";
    }
    fs << "      end\n";
    is_first = false;
  }

  // Arbitration to EN
  string high_en;
  for (IResource *caller : callers) {
    string a = TaskAckPin(*(tab_.GetITable()), caller->GetTable());
    rs << "  assign " << a << " = ";
    string en = TaskEnablePin(*(tab_.GetITable()), caller->GetTable());
    rs << en << " && ";
    if (!high_en.empty()) {
      // Ack will be asserted only when higher caller is not requesting.
      rs << "!(" << high_en << ") && ";
    }
    rs << ack << ";\n";
    // Appends this caller.
    if (!high_en.empty()) {
      high_en = high_en + " | ";
    }
    high_en += en;
  }
}

void Task::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  CHECK(resource::IsTask(*klass));
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
