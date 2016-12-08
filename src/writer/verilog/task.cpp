#include "writer/verilog/task.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
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
}

void Task::BuildTaskResource() {
  auto &conn = tab_.GetModule()->GetConnection();
  auto *callers = conn.GetTaskCallers(&res_);
  if (callers == nullptr) {
    return;
  }
  vector<string> task_en;
  for (IResource *caller : *callers) {
    BuildCallWire(caller);
    task_en.push_back(TaskEnablePin(*(tab_.GetITable()), caller->GetTable()));
  }
  string w = TaskEnablePin(*(tab_.GetITable()), nullptr);
  ostream &rs = tmpl_->GetStream(kResourceSection);
  rs << "  wire " << w << ";\n";
  rs << "  assign " << w << " = " << Util::Join(task_en, " | ") << ";\n";
  string ack = TaskAckPin(*(tab_.GetITable()), nullptr);
  ostream &fs = tab_.StateOutputSectionStream();
  fs << "      " << ack
     << " <= (" << tab_.StateVariable() << " == `"
     << tab_.StateName(Task::kTaskEntryStateId) << ") && " << w
     << ";\n";
  rs << "  reg " << ack << ";\n";
  ostream &is = tab_.InitialValueSectionStream();
  is << "  " << ack << " <= 0;\n";
  for (IResource *caller : *callers) {
    string a = TaskAckPin(*(tab_.GetITable()), caller->GetTable());
    rs << "  assign " << a << " = " << ack << ";\n";
  }
}

void Task::BuildCallWire(IResource *caller) {
  IModule *callee_module = res_.GetTable()->GetModule();
  IModule *caller_module = caller->GetTable()->GetModule();
  const IModule *common_root = Connection::GetCommonRoot(callee_module,
							 caller_module);
  if (caller_module == common_root) {
    // downward
    for (IModule *imod = callee_module; imod != common_root;
	 imod = imod->GetParentModule()) {
      AddPort(imod, caller, false);
    }
  } else if (callee_module == common_root) {
    // upward
    for (IModule *imod = caller_module; imod != common_root;
	 imod = imod->GetParentModule()) {
      AddPort(imod, caller, true);
    }
    AddWire(common_root, caller);
  } else {
    // downward
    for (IModule *imod = caller_module; imod != common_root;
	 imod = imod->GetParentModule()) {
      AddPort(imod, caller, true);
    }
    // upward
    for (IModule *imod = callee_module; imod != common_root;
	 imod = imod->GetParentModule()) {
      AddPort(imod, caller, false);
    }
    AddWire(common_root, caller);
  }
}

void Task::AddWire(const IModule *imod, IResource *caller) {
  Module *mod = tab_.GetModule()->GetByIModule(imod);
  Ports *ports = mod->GetPorts();
  string en = TaskEnablePin(*(tab_.GetITable()), caller->GetTable());
  string ack = TaskAckPin(*(tab_.GetITable()), caller->GetTable());
  auto *tmpl = mod->GetModuleTemplate();
  ostream &rs = tmpl->GetStream(kResourceSection);
  rs << "  wire " << en << ";\n";
  rs << "  wire " << ack << ";\n";
}

void Task::AddPort(const IModule *imod, IResource *caller, bool upward) {
  Module *mod = tab_.GetModule()->GetByIModule(imod);
  Ports *ports = mod->GetPorts();
  string en = TaskEnablePin(*(tab_.GetITable()), caller->GetTable());
  if (upward) {
    ports->AddPort(en, Port::OUTPUT_WIRE, 0);
  } else {
    ports->AddPort(en, Port::INPUT, 0);
  }
  Module *parent_mod = mod->GetParentModule();
  ostream &os = parent_mod->ChildModuleInstSectionStream(mod);
  os << ", ." << en << "(" << en << ")";
  string ack = TaskAckPin(*(tab_.GetITable()), caller->GetTable());
  if (upward) {
    ports->AddPort(ack, Port::INPUT, 0);
  } else {
    ports->AddPort(ack, Port::OUTPUT_WIRE, 0);
  }
  os << ", ." << ack << "(" << ack << ")";
}

void Task::BuildTaskCallResource() {
  ostream &rs = tmpl_->GetStream(kResourceSection);
  string en = TaskEnablePin(*(res_.GetCalleeTable()), tab_.GetITable());
  rs << "  wire " << en << ";\n";
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);
  rs << "  assign " << en << " = " << JoinStatesWithSubState(callers, 0)
     << ";\n";

  string ack = TaskAckPin(*(res_.GetCalleeTable()), tab_.GetITable());
  rs << "  wire " << ack << ";\n";
}

void Task::BuildTaskCallInsn(IInsn *insn, State *st) {
  ostream &os = st->StateBodySectionStream();
  static const char I[] = "          ";
  string st_name = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  string ack = TaskAckPin(*(res_.GetCalleeTable()), tab_.GetITable());
  os << I << "if (" << st_name << " == 0) begin\n"
     << I << "  if (" << ack << ") begin\n"
     << I << "    " << st_name << " <= 3;\n"
     << I << "  end else begin\n"
     << I << "  end\n"
     << I << "end\n";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
