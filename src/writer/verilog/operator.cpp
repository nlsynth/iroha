#include "writer/verilog/operator.h"

#include "iroha/logging.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

Operator::Operator(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void Operator::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsExclusiveBinOp(*klass)) {
    BuildExclusiveBinOp();
  }
}

void Operator::BuildInsn(IInsn *insn) {
  auto *klass = res_.GetClass();
  if (resource::IsLightBinOp(*klass)) {
    BuildLightBinOpInsn(insn);
  } else if (resource::IsBitArrangeOp(*klass)) {
    BuildBitArrangeOpInsn(insn);
  }
}

void Operator::BuildExclusiveBinOp() {
  ostream &rs = tmpl_->GetStream(kResourceSection);
  const string &res_name = res_.GetClass()->GetName();
  rs << "  // " << res_name << ":" << res_.GetId() << "\n";
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);
  if (callers.size() == 0) {
    return;
  }
  string name = InsnWriter::ResourceName(res_);
  WriteInputSel(name + "_s0", callers, 0, rs);
  WriteInputSel(name + "_s1", callers, 1, rs);
  WriteWire(name + "_d0", res_.output_types_[0], rs);

  rs << "  assign " << name << + "_d0 = "
     << name + "_s0 ";
  if (res_name == resource::kGt) {
    rs << ">";
  } else if (res_name == resource::kAdd) {
    rs << "+";
  } else if (res_name == resource::kSub) {
    rs << "-";
  } else {
    LOG(FATAL) << "Unknown binop" << res_name;
  }
  rs << " " << name + "_s1;\n";
}

void Operator::BuildLightBinOpInsn(IInsn *insn) {
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
      << " = " << InsnWriter::RegisterName(*insn->inputs_[0]) << " ";
  const string &rc = insn->GetResource()->GetClass()->GetName();
  if (rc == resource::kBitAnd) {
    ws << "&";
  } else if (rc == resource::kBitOr) {
    ws << "|";
  } else if (rc == resource::kBitXor) {
    ws << "^";
  } else {
    LOG(FATAL) << "Unknown LightBinOp: " << rc;
  }
  ws << " " << InsnWriter::RegisterName(*insn->inputs_[1]) << ";\n";
}

void Operator::BuildBitArrangeOpInsn(IInsn *insn) {
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  const string &rc = insn->GetResource()->GetClass()->GetName();
  if (rc == resource::kShift) {
    bool is_left = (insn->GetOperand() == "left");
    const IValue &value = insn->inputs_[1]->GetInitialValue();
    int amount = value.value_;
    ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
	<< " = "
	<< InsnWriter::RegisterName(*insn->inputs_[0]);
    if (is_left) {
      ws << " << ";
    } else {
      ws << " >> ";
    }
    ws << amount << ";\n";
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
