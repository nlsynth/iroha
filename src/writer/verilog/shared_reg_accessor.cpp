#include "writer/verilog/shared_reg_accessor.h"

#include "design/design_util.h"
#include "iroha/insn_operands.h"
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
    auto *params = res_.GetParentResource()->GetParams();
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
  ostream &rs = tab_.ResourceSectionStream();
  rs << "  // shared-reg-reader\n";
  const IResource *reg = res_.GetParentResource();
  if (UseMailbox(&res_)) {
    map<IState *, IInsn *> getters;
    CollectResourceCallers(operand::kGetMailbox, &getters);
    rs << "  assign " << SharedReg::RegMailboxGetReqName(res_) << " = "
       << JoinStatesWithSubState(getters, 0) << ";\n";
  }
  if (UseMailbox(&res_) || UseNotify(&res_)) {
    int width = res_.GetParentResource()->GetParams()->GetWidth();
    rs << "  reg " << Table::WidthSpec(width) << " "
       << SharedReg::RegMailboxBufName(res_) << ";\n";
  }
}

void SharedRegAccessor::BuildSharedRegWriterResource() {
  ostream &rs = tab_.ResourceSectionStream();
  rs << "  // shared-reg-writer\n";
  // Write en signal.
  map<IState *, IInsn *> callers;
  CollectResourceCallers("*", &callers);
  rs << "  assign " << SharedReg::WriterEnName(res_) << " = ";
  WriteStateUnion(callers, rs);
  rs << ";\n";
  rs << "  assign " << SharedReg::WriterName(res_) << " = ";
  string v;
  for (auto &p : callers) {
    if (v.empty()) {
      v = InsnWriter::InsnSpecificWireName(*(p.second));
    } else {
      v = "(" + tab_.GetStateCondition(p.first) + ") ? " +
	InsnWriter::InsnSpecificWireName(*(p.second)) + " : (" + v + ")";
    }
  }
  rs << v << ";\n";
  // write notify signal.
  if (UseNotify(&res_)) {
    map<IState *, IInsn *> notifiers;
    CollectResourceCallers(operand::kNotify, &notifiers);
    rs << "  assign " << SharedReg::WriterNotifierName(res_) << " = ";
    WriteStateUnion(notifiers, rs);
    rs << ";\n";
  }
  if (UseMailbox(&res_)) {
    map<IState *, IInsn *> putters;
    CollectResourceCallers(operand::kPutMailbox, &putters);
    rs << "  assign " << SharedReg::RegMailboxPutReqName(res_) << " = "
       << JoinStatesWithSubState(putters, 0) << ";\n";
  }
}

void SharedRegAccessor::GetAccessorFeatures(const IResource *accessor,
					    bool *use_notify,
					    bool *use_mailbox) {
  *use_notify = false;
  *use_mailbox = false;
  auto *klass = accessor->GetClass();
  if (resource::IsDataFlowIn(*klass)) {
    *use_notify = true;
    return;
  }
  vector<IInsn *> insns = DesignUtil::GetInsnsByResource(accessor);
  for (auto *insn : insns) {
    const string &op = insn->GetOperand();
    // reader and writer respectively.
    if (op == operand::kWaitNotify ||
	op == operand::kNotify) {
      *use_notify = true;
    }
    // ditto.
    if (op == operand::kGetMailbox ||
	op == operand::kPutMailbox) {
      *use_mailbox = true;
    }
  }
}

void SharedRegAccessor::BuildReadInsn(IInsn *insn, State *st) {
  IResource *source = res_.GetParentResource();
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  int s = 0;
  for (int i = 0; i < insn->outputs_.size(); ++i) {
    ws << "  assign "
       << InsnWriter::InsnOutputWireName(*insn, i)
       << " = ";
    if (insn->GetOperand() == operand::kGetMailbox ||
	insn->GetOperand() == operand::kWaitNotify) {
      ws << SharedReg::RegMailboxBufName(res_);
    } else {
      ws << SharedReg::RegName(*source);
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
  static const char I[] = "          ";
  string insn_st = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  ostream &os = st->StateBodySectionStream();
  if (insn->GetOperand() == operand::kWaitNotify) {
    os << I << "// Wait notify\n"
       << I << "if (" << insn_st << " == 0) begin\n"
       << I << "  if (" << SharedReg::RegNotifierName(*source) << ") begin\n"
       << I << "    " << SharedReg::RegMailboxBufName(res_) << " <= "
       << SharedReg::RegName(*source) << ";\n"
       << I << "    " << insn_st << " <= 3;\n"
       << I << "  end\n"
       << I << "end\n";
  }
  if (insn->GetOperand() == operand::kGetMailbox) {
    os << I << "// Wait get mailbox\n"
       << I << "if (" << insn_st << " == 0) begin\n"
       << I << "  if (" << SharedReg::RegMailboxGetAckName(res_) << ") begin\n"
       << I << "    " << SharedReg::RegMailboxBufName(res_) << " <= "
       << SharedReg::RegName(*source) << ";\n"
       << I << "    " << insn_st << " <= 3;\n"
       << I << "  end\n"
       << I << "end\n";
  }
}

void SharedRegAccessor::BuildWriteInsn(IInsn *insn, State *st) {
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  int width = res_.GetParentResource()->GetParams()->GetWidth();
  ws << "  wire " << Table::WidthSpec(width)
     << InsnWriter::InsnSpecificWireName(*insn) << ";\n";
  int s = 0;
  for (int i = 0; i < insn->inputs_.size(); ++i) {
    ws << "  assign " << InsnWriter::InsnSpecificWireName(*insn);
    if (insn->inputs_.size() > 1) {
      IRegister *reg = insn->inputs_[i];
      int w = reg->value_type_.GetWidth();
      if (w == 0) {
	w = 1;
      }
      ws << "[" << (s + w - 1) << ":" << s << "]";
      s += w;
    }
    ws << " = "
       << InsnWriter::RegisterValue(*insn->inputs_[i], tab_.GetNames())
       << ";\n";
  }
  if (insn->GetOperand() == operand::kPutMailbox) {
    static const char I[] = "          ";
    string insn_st = InsnWriter::MultiCycleStateName(res_);
    ostream &os = st->StateBodySectionStream();
    os << I << "// Wait put mailbox\n"
       << I << "if (" << insn_st << " == 0) begin\n"
       << I << "  if (" << SharedReg::RegMailboxPutAckName(res_) << ") begin\n"
       << I << "    " << insn_st << " <= 3;\n"
       << I << "  end\n"
       << I << "end\n";  }
}

bool SharedRegAccessor::UseNotify(const IResource *accessor) {
  bool n, s;
  GetAccessorFeatures(accessor, &n, &s);
  return n;
}

bool SharedRegAccessor::UseMailbox(const IResource *accessor) {
  bool n, s;
  GetAccessorFeatures(accessor, &n, &s);
  return s;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
