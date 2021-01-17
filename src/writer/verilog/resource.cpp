#include "writer/verilog/resource.h"

#include <set>

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/array.h"
#include "writer/verilog/array_rdata.h"
#include "writer/verilog/axi/master_port.h"
#include "writer/verilog/axi/slave_port.h"
#include "writer/verilog/dataflow_in.h"
#include "writer/verilog/debug_resource.h"
#include "writer/verilog/embedded_modules.h"
#include "writer/verilog/ext_combinational.h"
#include "writer/verilog/ext_io.h"
#include "writer/verilog/ext_io_accessor.h"
#include "writer/verilog/ext_task.h"
#include "writer/verilog/ext_task_call.h"
#include "writer/verilog/fifo.h"
#include "writer/verilog/fifo_accessor.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/operator.h"
#include "writer/verilog/port.h"
#include "writer/verilog/port_set.h"
#include "writer/verilog/shared_memory.h"
#include "writer/verilog/shared_memory_accessor.h"
#include "writer/verilog/shared_memory_replica.h"
#include "writer/verilog/shared_reg.h"
#include "writer/verilog/shared_reg_accessor.h"
#include "writer/verilog/shared_reg_ext_writer.h"
#include "writer/verilog/sram_if.h"
#include "writer/verilog/state.h"
#include "writer/verilog/study.h"
#include "writer/verilog/study_accessor.h"
#include "writer/verilog/table.h"
#include "writer/verilog/task.h"
#include "writer/verilog/task_call.h"
#include "writer/verilog/ticker.h"
#include "writer/verilog/ticker_accessor.h"

namespace iroha {
namespace writer {
namespace verilog {

Resource *Resource::Create(const IResource &res, const Table &table) {
  auto *klass = res.GetClass();
  if (resource::IsTask(*klass)) {
    return new Task(res, table);
  }
  if (resource::IsTaskCall(*klass)) {
    return new TaskCall(res, table);
  }
  if (resource::IsExclusiveBinOp(*klass) || resource::IsLightBinOp(*klass) ||
      resource::IsLightUniOp(*klass) || resource::IsBitShiftOp(*klass) ||
      resource::IsBitSel(*klass) || resource::IsBitConcat(*klass) ||
      resource::IsSelect(*klass)) {
    return new Operator(res, table);
  }
  if (resource::IsExtInput(*klass) || resource::IsExtOutput(*klass)) {
    return new ExtIO(res, table);
  }
  if (resource::IsExtInputAccessor(*klass) ||
      resource::IsExtOutputAccessor(*klass)) {
    return new ExtIOAccessor(res, table);
  }
  if (resource::IsSharedReg(*klass)) {
    return new SharedReg(res, table);
  }
  if (resource::IsSharedRegReader(*klass) ||
      resource::IsSharedRegWriter(*klass)) {
    return new SharedRegAccessor(res, table);
  }
  if (resource::IsSharedRegExtWriter(*klass)) {
    return new SharedRegExtWriter(res, table);
  }
  if (resource::IsSharedMemory(*klass)) {
    return new SharedMemory(res, table);
  }
  if (resource::IsSharedMemoryReader(*klass) ||
      resource::IsSharedMemoryWriter(*klass)) {
    return new SharedMemoryAccessor(res, table);
  }
  if (resource::IsSharedMemoryReplica(*klass)) {
    return new SharedMemoryReplica(res, table);
  }
  if (resource::IsSramIf(*klass)) {
    return new SramIf(res, table);
  }
  if (resource::IsArray(*klass)) {
    return new ArrayResource(res, table);
  }
  if (resource::IsArrayRData(*klass)) {
    return new ArrayRDataResource(res, table);
  }
  if (resource::IsAxiMasterPort(*klass)) {
    return new axi::MasterPort(res, table);
  }
  if (resource::IsAxiSlavePort(*klass)) {
    return new axi::SlavePort(res, table);
  }
  if (resource::IsExtCombinational(*klass)) {
    return new ExtCombinational(res, table);
  }
  if (resource::IsExtTask(*klass) || resource::IsExtTaskDone(*klass)) {
    return new ExtTask(res, table);
  }
  if (resource::IsFifo(*klass)) {
    return new Fifo(res, table);
  }
  if (resource::IsFifoReader(*klass) || resource::IsFifoWriter(*klass)) {
    return new FifoAccessor(res, table);
  }
  if (resource::IsExtTaskCall(*klass) || resource::IsExtTaskWait(*klass) ||
      resource::IsExtFlowCall(*klass) || resource::IsExtFlowResult(*klass)) {
    return new ExtTaskCall(res, table);
  }
  if (resource::IsTicker(*klass)) {
    return new Ticker(res, table);
  }
  if (resource::IsTickerAccessor(*klass)) {
    return new TickerAccessor(res, table);
  }
  if (resource::IsDataFlowIn(*klass)) {
    return new DataFlowIn(res, table);
  }
  if (resource::IsStudy(*klass)) {
    return new Study(res, table);
  }
  if (resource::IsStudyReader(*klass) || resource::IsStudyWriter(*klass)) {
    return new StudyAccessor(res, table);
  }
  if (resource::IsPrint(*klass) || resource::IsAssert(*klass)) {
    return new DebugResource(res, table);
  }
  return new Resource(res, table);
}

Resource::Resource(const IResource &res, const Table &table)
    : res_(res), tab_(table) {
  tmpl_ = tab_.GetModuleTemplate();
}

Resource::~Resource() {}

void Resource::BuildResource() {}

void Resource::BuildInsn(IInsn *insn, State *st) {
  auto *rc = res_.GetClass();
  if (resource::IsSet(*rc)) {
    BuildAssignInsn(insn, st);
  }
}

void Resource::BuildAssignInsn(IInsn *insn, State *st) {
  if (insn->outputs_[0]->IsStateLocal()) {
    ostream &ws = tab_.InsnWireValueSectionStream();
    ws << "  assign " << insn->outputs_[0]->GetName() << " = "
       << InsnWriter::RegisterValue(*insn->inputs_[0], st->GetNames()) << ";\n";
  } else {
    ostream &os = st->StateBodySectionStream();
    IRegister *lhs = insn->outputs_[0];
    os << "          " << lhs->GetName() << " <= "
       << InsnWriter::RegisterValueWithConstWidth(
              *insn->inputs_[0], lhs->value_type_.GetWidth(), st->GetNames())
       << ";\n";
  }
}

void Resource::CollectNames(Names *names) {}

void Resource::CollectResourceCallers(const string &opr,
                                      map<IState *, IInsn *> *callers) const {
  vector<string> v;
  Util::SplitStringUsing(opr, ",", &v);
  set<string> oprs;
  for (auto &o : v) {
    oprs.insert(o);
  }
  if (opr.empty()) {
    oprs.insert("");
  }
  for (auto *st : res_.GetTable()->states_) {
    for (auto *insn : st->insns_) {
      if (insn->GetResource() == &res_ &&
          (opr == "*" || oprs.find(insn->GetOperand()) != oprs.end())) {
        callers->insert(make_pair(st, insn));
      }
    }
  }
}

void Resource::WriteInputSel(const string &name,
                             const map<IState *, IInsn *> &callers, int nth,
                             ostream &os) {
  WriteWire(name, res_.input_types_[nth], os);
  os << "  assign " << name << " = ";

  string cond;
  for (auto &it : callers) {
    IInsn *insn = it.second;
    IState *st = it.first;
    if (cond.empty()) {
      cond = InsnWriter::RegisterValue(*insn->inputs_[nth], tab_.GetNames());
    } else {
      cond = "(" + tab_.StateVariable() + " == " + tab_.StateName(st->GetId()) +
             ") ? " +
             InsnWriter::RegisterValue(*insn->inputs_[nth], tab_.GetNames()) +
             " : (" + cond + ")";
    }
  }
  os << cond << ";\n";
}

void Resource::WriteWire(const string &name, const IValueType &type,
                         ostream &os) {
  os << "  wire ";
  if (type.IsSigned()) {
    os << "signed ";
  }
  int width = type.GetWidth();
  if (width > 0) {
    os << "[" << (width - 1) << ":0] ";
  }
  os << name << ";\n";
}

void Resource::WriteStateUnion(const map<IState *, IInsn *> &callers,
                               ostream &os) {
  if (callers.size() == 0) {
    os << "0";
  }
  bool is_first = true;
  for (auto &c : callers) {
    if (!is_first) {
      os << " | ";
    }
    os << "(" << tab_.StateVariable() << " == " << c.first->GetId() << ")";
    is_first = false;
  }
}

string Resource::JoinStates(const map<IState *, IInsn *> &sts) const {
  vector<string> conds;
  for (auto &p : sts) {
    IState *st = p.first;
    conds.push_back("(" + tab_.StateVariable() + " == " +
                    Table::StateNameFromTable(*tab_.GetITable(), st->GetId()) +
                    ")");
  }
  return Util::Join(conds, " || ");
}

string Resource::JoinStatesWithSubState(const map<IState *, IInsn *> &sts,
                                        int sub) const {
  vector<string> conds;
  for (auto &p : sts) {
    IState *st = p.first;
    IInsn *insn = p.second;
    conds.push_back("(" + tab_.GetStateCondition(st) + " && " +
                    InsnWriter::MultiCycleStateName(*insn->GetResource()) +
                    " == " + Util::Itoa(sub) + ")");
  }
  return Util::Join(conds, " || ");
}

string Resource::SelectValueByStateWithCallers(
    const map<IState *, IInsn *> &callers, const string &default_value) {
  string v = default_value;
  for (auto &c : callers) {
    IState *st = c.first;
    IInsn *insn = c.second;
    if (insn->inputs_.size() == 1) {
      string stCond = tab_.GetStateCondition(st);
      v = "((" + stCond + ") ? " +
          InsnWriter::RegisterValue(*insn->inputs_[0], tab_.GetNames()) +
          " : " + v + ")";
    }
  }
  return v;
}

string Resource::SelectValueByState(const string &default_value) {
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);
  return SelectValueByStateWithCallers(callers, default_value);
}

void Resource::AddPortToTop(const string &port, bool is_output,
                            bool from_embedded, int width) {
  // Owner module.
  Port::PortType type;
  if (is_output) {
    type = Port::OUTPUT;
    if (from_embedded) {
      type = Port::OUTPUT_WIRE;
    }
  } else {
    type = Port::INPUT;
  }
  auto *ports = tab_.GetPortSet();
  ports->AddPort(port, type, width);
  // Outer modules.
  if (is_output) {
    type = Port::OUTPUT_WIRE;
  }
  for (Module *mod = tab_.GetModule(); mod != nullptr;
       mod = mod->GetParentModule()) {
    Module *parent_mod = mod->GetParentModule();
    if (parent_mod != nullptr) {
      ostream &os = parent_mod->ChildModuleInstSectionStream(mod);
      os << ", ." << port << "(" << port << ")";
      auto *parent_ports = parent_mod->GetPortSet();
      parent_ports->AddPort(port, type, width);
    }
  }
}

ModuleTemplate *Resource::GetModuleTemplate() const { return tmpl_; }

const Table &Resource::GetTable() const { return tab_; }

const IResource &Resource::GetIResource() const { return res_; }

void Resource::BuildEmbeddedModule(const string &connection) {
  auto *params = res_.GetParams();
  tab_.GetEmbeddedModules()->RequestModule(*params);

  auto *ports = tab_.GetPortSet();
  ostream &is = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = params->GetEmbeddedModuleName();
  is << "  // " << name << "\n"
     << "  " << name << " inst_" << tab_.GetITable()->GetId() << "_"
     << res_.GetId() << "_" << name << "(";
  is << "." << params->GetEmbeddedModuleClk() << "(" << ports->GetClk() << "), "
     << "." << params->GetEmbeddedModuleReset() << "(" << ports->GetReset()
     << ")";
  is << connection;
  is << ");\n";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
