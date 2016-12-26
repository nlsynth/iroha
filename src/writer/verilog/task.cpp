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
    if (resource::IsTask(*(task_entry_insn->GetResource()->GetClass()))) {
      return true;
    }
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
  string s = "task_" + Util::Itoa(tab.GetId());
  if (caller != nullptr) {
    IModule *caller_mod = caller->GetModule();
    s += "_" + Util::Itoa(caller_mod->GetId()) +
      "_" + Util::Itoa(caller->GetId());
  }
  return s;
}

void Task::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsTask(*klass)) {
    BuildTaskResource();
  }
  if (resource::IsTaskCall(*klass)) {
    BuildTaskCallResource();
  }
}

void Task::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsTaskCall(*klass)) {
    BuildTaskCallInsn(insn, st);
  }
  if (resource::IsTaskCall(*klass)) {
    BuildTaskInsn(insn, st);
  }
}

void Task::BuildTaskResource() {
  auto &conn = tab_.GetModule()->GetConnection();
  auto *callers = conn.GetTaskCallers(&res_);
  if (callers == nullptr) {
    return;
  }
  // Inter module wires.
  for (IResource *caller : *callers) {
    BuildCallWire(caller);
  }
  // EN
  vector<string> task_en;
  for (IResource *caller : *callers) {
    task_en.push_back(TaskEnablePin(*(tab_.GetITable()), caller->GetTable()));
  }
  string common_en = TaskEnablePin(*(tab_.GetITable()), nullptr);
  ostream &rs = tmpl_->GetStream(kResourceSection);
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
  for (IResource *caller : *callers) {
    string e;
    if (!is_first) {
      e = "else ";
    }
    string en = TaskEnablePin(*(tab_.GetITable()), caller->GetTable());
    fs << "     " << e << "if (" << ack_cond << " && " << en << ") begin\n";
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
  for (IResource *caller : *callers) {
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

void Task::BuildCallWire(IResource *caller) {
  IModule *callee_module = res_.GetTable()->GetModule();
  IModule *caller_module = caller->GetTable()->GetModule();
  const IModule *common_root = Connection::GetCommonRoot(callee_module,
							 caller_module);
  if (caller_module != common_root) {
    AddWire(common_root, caller);
  }
  // downward
  for (IModule *imod = callee_module; imod != common_root;
       imod = imod->GetParentModule()) {
    AddPort(imod, caller, false);
  }
  // upward
  for (IModule *imod = caller_module; imod != common_root;
       imod = imod->GetParentModule()) {
    AddPort(imod, caller, true);
  }
}

void Task::AddWire(const IModule *imod, IResource *caller) {
  Module *mod = tab_.GetModule()->GetByIModule(imod);
  string en = TaskEnablePin(*(tab_.GetITable()), caller->GetTable());
  string ack = TaskAckPin(*(tab_.GetITable()), caller->GetTable());
  auto *tmpl = mod->GetModuleTemplate();
  ostream &rs = tmpl->GetStream(kResourceSection);
  rs << "  wire " << en << ";\n";
  rs << "  wire " << ack << ";\n";
  for (int i = 0; i < res_.output_types_.size(); ++i) {
    auto &type = res_.output_types_[i];
    rs << "  wire " << Table::ValueWidthSpec(type) << " "
       << TaskArgPin(*(tab_.GetITable()), i, false, caller->GetTable())
       << ";\n";
  }
}

void Task::AddPort(const IModule *imod, IResource *caller, bool upward) {
  Module *mod = tab_.GetModule()->GetByIModule(imod);
  Ports *ports = mod->GetPorts();
  // EN
  string en = TaskEnablePin(*(tab_.GetITable()), caller->GetTable());
  if (upward) {
    ports->AddPort(en, Port::OUTPUT_WIRE, 0);
  } else {
    ports->AddPort(en, Port::INPUT, 0);
  }
  Module *parent_mod = mod->GetParentModule();
  ostream &os = parent_mod->ChildModuleInstSectionStream(mod);
  os << ", ." << en << "(" << en << ")";
  // ACK
  string ack = TaskAckPin(*(tab_.GetITable()), caller->GetTable());
  if (upward) {
    ports->AddPort(ack, Port::INPUT, 0);
  } else {
    ports->AddPort(ack, Port::OUTPUT_WIRE, 0);
  }
  os << ", ." << ack << "(" << ack << ")";
  // Args
  for (int i = 0; i < res_.output_types_.size(); ++i) {
    auto &type = res_.output_types_[i];
    string a = TaskArgPin(*(tab_.GetITable()), i, false, caller->GetTable());
    int w = type.GetWidth();
    if (upward) {
      ports->AddPort(a, Port::OUTPUT_WIRE, w);
    } else {
      ports->AddPort(a, Port::INPUT, w);
    }
    os << ", ." << a << "(" << a << ")";
  }
}

void Task::BuildTaskCallResource() {
  ostream &rs = tmpl_->GetStream(kResourceSection);
  string en = TaskEnablePin(*(res_.GetCalleeTable()), tab_.GetITable());
  rs << "  reg " << en << ";\n";
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);
  ostream &ss = tab_.StateOutputSectionStream();
  ss << "      " << en << " <= " << JoinStatesWithSubState(callers, 0)
     << ";\n";
  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << en << " <= 0;\n";

  ITable *callee = res_.GetCalleeTable();
  string ack = TaskAckPin(*callee, tab_.GetITable());
  rs << "  wire " << ack << ";\n";
  for (int i = 0; i < res_.input_types_.size(); ++i) {
    auto &type = res_.input_types_[i];
    rs << "  reg " << Table::ValueWidthSpec(type) << " "
       << TaskArgPin(*callee, i, false, res_.GetTable()) << ";\n";
  }
}

void Task::BuildTaskInsn(IInsn *insn, State *st) {
}

void Task::BuildTaskCallInsn(IInsn *insn, State *st) {
  ostream &os = st->StateBodySectionStream();
  static const char I[] = "          ";
  string st_name = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  string ack = TaskAckPin(*(res_.GetCalleeTable()), tab_.GetITable());
  os << I << "if (" << st_name << " == 0) begin\n";
  ITable *callee = res_.GetCalleeTable();
  CHECK(res_.input_types_.size() == insn->inputs_.size());
  for (int i = 0; i < res_.input_types_.size(); ++i) {
    os << I << "  " << TaskArgPin(*callee, i, false, res_.GetTable())
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
