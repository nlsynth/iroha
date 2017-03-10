#include "opt/array_to_mem.h"

#include <map>

#include "design/design_tool.h"
#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/insn_operands.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"

namespace iroha {
namespace opt {

static const string name("array_to_mem");

ArrayToMem::~ArrayToMem() {
}

Phase *ArrayToMem::Create() {
  return new ArrayToMem();
}

bool ArrayToMem::ApplyForTable(ITable *table) {
  map<IResource *, IResource *> array_to_mem;
  auto copied_res = table->resources_;
  for (IResource *res : copied_res) {
    if (res->GetClass()->GetName() == resource::kArray) {
      IResource *mem_res = GetResource(res);
      array_to_mem[res] = mem_res;
    }
  }
  struct ArrayInsn {
    IState *st;
    IInsn *insn;
  };
  vector<ArrayInsn> insns;
  for (IState *st : table->states_) {
    for (IInsn *insn : st->insns_) {
      if (insn->GetResource()->GetClass()->GetName() == resource::kArray) {
	ArrayInsn ai;
	ai.st = st;
	ai.insn = insn;
	insns.push_back(ai);
      }
    }
  }
  for (ArrayInsn &ai : insns) {
    IResource *mem = array_to_mem[ai.insn->GetResource()];
    AddMemInsn(mem, ai.st, ai.insn);
  }
  for (ArrayInsn &ai : insns) {
    DeleteArrayInsn(ai.st, ai.insn);
  }
  return true;
}

void ArrayToMem::AddMemInsn(IResource *mem, IState *st, IInsn *array_insn) {
  if (array_insn->outputs_.size() == 0) {
    // Write.
    IInsn *insn = new IInsn(mem);
    insn->SetOperand(operand::kSramWrite);
    st->insns_.push_back(insn);
    insn->inputs_ = array_insn->inputs_;
  } else {
    // Read.
    IInsn *addr_insn = new IInsn(mem);
    addr_insn->SetOperand(operand::kSramReadAddress);
    addr_insn->inputs_ = array_insn->inputs_;
    st->insns_.push_back(addr_insn);
    // Data phase
    IState *data_st = DesignTool::InsertNextState(st);
    IInsn *data_insn = new IInsn(mem);
    data_insn->SetOperand(operand::kSramReadData);
    data_insn->outputs_ = array_insn->outputs_;
    data_st->insns_.push_back(data_insn);
  }
}

void ArrayToMem::DeleteArrayInsn(IState *st, IInsn *array_insn) {
  DesignTool::DeleteInsn(st, array_insn);
}

IResource *ArrayToMem::GetResource(IResource *array_res) {
  ITable *table = array_res->GetTable();
  for (auto *res : table->resources_) {
    if (res->GetArray() == array_res->GetArray() &&
	res->GetClass()->GetName() == resource::kMapped) {
      auto *param = res->GetParams();
      if (param->GetMappedName() == "mem") {
	return res;
      }
    }
  }
  IResource *res = DesignUtil::CreateResource(table, resource::kMapped);
  auto *param = res->GetParams();
  param->SetMappedName("mem");
  res->SetArray(array_res->GetArray());
  return res;
}

}  // namespace opt
}  // namespace iroha
