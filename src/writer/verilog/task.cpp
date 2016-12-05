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
  string s = "task_" + Util::Itoa(tab.GetId());
  if (caller != nullptr) {
    IModule *caller_mod = caller->GetModule();
    s += "_" + Util::Itoa(caller_mod->GetId()) +
      "_" + Util::Itoa(caller->GetId());
  }
  return s + "_en";
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
}

void Task::BuildCallWire(IResource *caller) {
  IModule *callee_module = res_.GetTable()->GetModule();
  IModule *caller_module = caller->GetTable()->GetModule();
  const IModule *common_root = Connection::GetCommonRoot(callee_module,
							 caller_module);
  if (caller_module == common_root) {
    for (IModule *imod = callee_module; imod != common_root;
	 imod = imod->GetParentModule()) {
      AddDownwardPort(imod, caller);
    }
  }
}

void Task::AddDownwardPort(IModule *imod, IResource *caller) {
  Module *mod = tab_.GetModule()->GetByIModule(imod);
  Ports *ports = mod->GetPorts();
  string en = TaskEnablePin(*(tab_.GetITable()), caller->GetTable());
  ports->AddPort(en, Port::INPUT, 0);
  Module *parent_mod = mod->GetParentModule();
  ostream &os = parent_mod->ChildModuleInstSectionStream(mod);
  os << ", ." << en << "(" << en << ")";
}

void Task::BuildTaskCallResource() {
  ostream &rs = tmpl_->GetStream(kResourceSection);
  string en = TaskEnablePin(*(res_.GetCalleeTable()), tab_.GetITable());
  rs << "  wire " << en << ";\n";
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);
  rs << "  assign " << en << " = " << JoinStatesWithSubState(callers, 0)
     << ";\n";
}

void Task::BuildTaskCallInsn(IInsn *insn, State *st) {
  ostream &os = st->StateBodySectionStream();
  static const char I[] = "          ";
  string st_name = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  os << I << "if (" << st_name << " == 0) begin\n"
     << I << "  " << st_name << " <= 3;\n"
     << I << "end\n";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
