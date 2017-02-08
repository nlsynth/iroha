#include "writer/verilog/shared_reg_accessor.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"
#include "writer/verilog/shared_reg.h"

namespace iroha {
namespace writer {
namespace verilog {

SharedRegAccessor::SharedRegAccessor(const IResource &res,
				     const Table &table)
  : Resource(res, table) {
  auto *klass = res_.GetClass();
  if (resource::IsSharedRegWriter(*klass)) {
    auto *params = res_.GetSharedRegister()->GetParams();
    string unused;
    params->GetExtOutputPort(&unused, &width_);
  } else {
    width_ = 0;
  }
}

void SharedRegAccessor::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsSharedRegWriter(*klass)) {
    BuildSharedRegWriterResource();
  }
  if (resource::IsSharedRegReader(*klass)) {
    BuildSharedRegReaderResource();
  }
}

void SharedRegAccessor::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsSharedRegReader(*klass)) {
    // Read from another table.
    BuildReadInsn(insn, st);
  }
  if (resource::IsSharedRegWriter(*klass)) {
    BuildWriteInsn(insn, st);
  }
}

void SharedRegAccessor::BuildSharedRegReaderResource() {
  if (UseSemaphore(&res_)) {
    ostream &rs = tmpl_->GetStream(kRegisterSection);
    ostream &is = tab_.InitialValueSectionStream();
    rs << "  // shared-reg-reader\n";
    rs << "  reg " << SharedReg::RegSemaphoreGetReqName(res_) << ";\n";
    is << "      " << SharedReg::RegSemaphoreGetReqName(res_) << " <= 0;\n";
    map<IState *, IInsn *> getters;
    CollectResourceCallers("get_semaphore", &getters);
    ostream &os = tab_.StateOutputSectionStream();
    os << "      " << SharedReg::RegSemaphoreGetReqName(res_) << " <= "
       << JoinStatesWithSubState(getters, 0) << ";\n";
  }
}

void SharedRegAccessor::BuildSharedRegWriterResource() {
  ostream &rs = tmpl_->GetStream(kRegisterSection);
  rs << "  // shared-reg-writer\n";
  rs << "  reg ";
  if (width_ > 0) {
    rs << "[" << width_ - 1 << ":0]";
  }
  rs << " " << SharedReg::WriterName(res_) << ";\n";
  rs << "  reg " << SharedReg::WriterEnName(res_) << ";\n";
  // Reset value
  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << SharedReg::WriterName(res_) << " <= 0;\n"
     << "      " << SharedReg::WriterEnName(res_) << " <= 0;\n";
  if (UseNotify(&res_)) {
    is << "      " << SharedReg::WriterNotifierName(res_) << " <= 0;\n";
  }
  // Notify and semaphore
  if (UseNotify(&res_)) {
    rs << "  reg " << SharedReg::WriterNotifierName(res_) << ";\n";
  }
  if (UseSemaphore(&res_)) {
    rs << "  reg " << SharedReg::RegSemaphorePutReqName(res_) << ";\n";
    is << "      " << SharedReg::RegSemaphorePutReqName(res_) << " <= 0;\n";
  }
  // Write en signal.
  ostream &os = tab_.StateOutputSectionStream();
  map<IState *, IInsn *> callers;
  CollectResourceCallers("*", &callers);
  os << "      " << SharedReg::WriterEnName(res_) << " <= ";
  WriteStateUnion(callers, os);
  os << ";\n";
  // write notify signal.
  if (UseNotify(&res_)) {
    map<IState *, IInsn *> notifiers;
    CollectResourceCallers("notify", &notifiers);
    os << "      " << SharedReg::WriterNotifierName(res_) << " <= ";
    WriteStateUnion(notifiers, os);
    os << ";\n";
  }
  if (UseSemaphore(&res_)) {
    map<IState *, IInsn *> putters;
    CollectResourceCallers("put_semaphore", &putters);
    os << "      " << SharedReg::RegSemaphorePutReqName(res_) << " <= "
       << JoinStatesWithSubState(putters, 0) << ";\n";
  }

  BuildWriteWire(&res_);
}

void SharedRegAccessor::BuildWriteWire(const IResource *writer) {
  IResource *reg = writer->GetSharedRegister();
  IModule *reg_module = reg->GetTable()->GetModule();
  IModule *writer_module = writer->GetTable()->GetModule();
  const IModule *common_root = Connection::GetCommonRoot(reg_module,
							 writer_module);
  if (writer_module != common_root) {
    SharedReg::AddWire(common_root, &tab_, writer, true);
  }
  // downward
  for (IModule *imod = reg_module; imod != common_root;
       imod = imod->GetParentModule()) {
    AddWritePort(imod, writer, false);
  }
  // upward
  for (IModule *imod = writer_module; imod != common_root;
       imod = imod->GetParentModule()) {
    AddWritePort(imod, writer, true);
  }
}

void SharedRegAccessor::AddWritePort(const IModule *imod,
				     const IResource *writer,
				     bool upward) {
  Module *mod = tab_.GetModule()->GetByIModule(imod);
  Ports *ports = mod->GetPorts();
  int width = writer->GetSharedRegister()->GetParams()->GetWidth();
  bool notify = UseNotify(writer);
  bool sem = UseSemaphore(writer);
  if (upward) {
    ports->AddPort(SharedReg::WriterName(*writer), Port::OUTPUT_WIRE, width);
    ports->AddPort(SharedReg::WriterEnName(*writer), Port::OUTPUT_WIRE, 0);
    if (notify) {
      ports->AddPort(SharedReg::WriterNotifierName(*writer), Port::OUTPUT_WIRE, 0);
    }
    if (sem) {
      ports->AddPort(SharedReg::RegSemaphoreGetReqName(*writer), Port::OUTPUT_WIRE, 0);
    }
  } else {
    ports->AddPort(SharedReg::WriterName(*writer), Port::INPUT, width);
    ports->AddPort(SharedReg::WriterEnName(*writer), Port::INPUT, 0);
    if (notify) {
      ports->AddPort(SharedReg::WriterNotifierName(*writer), Port::INPUT, 0);
    }
    if (sem) {
      ports->AddPort(SharedReg::RegSemaphoreGetReqName(*writer), Port::INPUT, 0);
    }
  }
  Module *parent_mod = mod->GetParentModule();
  ostream &os = parent_mod->ChildModuleInstSectionStream(mod);
  SharedReg::AddChildWire(writer, true, os);
}

void SharedRegAccessor::GetAccessorFeatures(const IResource *accessor,
					    bool *use_notify, bool *use_sem) {
  *use_notify = false;
  *use_sem = false;
  vector<IInsn *> insns = DesignUtil::GetInsnsByResource(accessor);
  for (auto *insn : insns) {
    const string &op = insn->GetOperand();
    // reader and writer respectively.
    if (op == "wait_notify" ||
	op == "notify") {
      *use_notify = true;
    }
    // ditto.
    if (op == "get_semaphore" ||
	op == "put_semaphore") {
      *use_sem = true;
    }
  }
}

void SharedRegAccessor::BuildReadInsn(IInsn *insn, State *st) {
  IResource *source = res_.GetSharedRegister();
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  ws << "  assign "
     << InsnWriter::InsnOutputWireName(*insn, 0)
     << " = "
     << SharedReg::RegName(*source) << ";\n";
  static const char I[] = "          ";
  string insn_st = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  ostream &os = st->StateBodySectionStream();
  if (insn->GetOperand() == "wait_notify") {
    os << I << "// Wait notify\n"
       << I << "if (" << insn_st << " == 0) begin\n"
       << I << "  if (" << SharedReg::RegNotifierName(*source) << ") begin\n"
       << I << "    " << insn_st << " <= 3;\n"
       << I << "  end\n"
       << I << "end\n";
  }
  if (insn->GetOperand() == "get_semaphore") {
    os << I << "// Wait get semaphore\n"
       << I << "if (" << insn_st << " == 0) begin\n"
       << I << "  if (" << SharedReg::RegSemaphoreGetAckName(res_) << ") begin\n"
       << I << "    " << insn_st << " <= 3;\n"
       << I << "  end\n"
       << I << "end\n";
  }
}

void SharedRegAccessor::BuildWriteInsn(IInsn *insn, State *st) {
  ostream &ss = st->StateBodySectionStream();
  ss << "          " << SharedReg::WriterName(res_) << " <= "
     << InsnWriter::RegisterValue(*insn->inputs_[0], tab_.GetNames())
     << ";\n";
  if (insn->GetOperand() == "put_semaphore") {
    static const char I[] = "          ";
    string insn_st = InsnWriter::MultiCycleStateName(res_);
    ostream &os = st->StateBodySectionStream();
    os << I << "// Wait put semaphore\n"
       << I << "if (" << insn_st << " == 0) begin\n"
       << I << "  if (" << SharedReg::RegSemaphorePutAckName(res_) << ") begin\n"
       << I << "    " << insn_st << " <= 3;\n"
       << I << "  end\n"
       << I << "end\n";  }
}

bool SharedRegAccessor::UseNotify(const IResource *accessor) {
  bool n, s;
  GetAccessorFeatures(accessor, &n, &s);
  return n;
}

bool SharedRegAccessor::UseSemaphore(const IResource *accessor) {
  bool n, s;
  GetAccessorFeatures(accessor, &n, &s);
  return s;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
