#include "opt/array_split_rdata.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"

namespace iroha {
namespace opt {

namespace {

class TableConverter {
public:
  TableConverter(ITable *tab);

  void Convert();

private:
  IResource *GetRDataResource(IResource *a);
  bool IsArrayRead(IInsn *insn);

  ITable *tab_;
  map<IResource *, IResource *> array_to_rdata_;
};

TableConverter::TableConverter(ITable *tab) : tab_(tab) {
}

void TableConverter::Convert() {
  for (IState *st : tab_->states_) {
    vector<IInsn *> new_insns;
    for (IInsn *insn : st->insns_) {
      if (!IsArrayRead(insn)) {
	new_insns.push_back(insn);
	continue;
      }
      // Replaces insn with rd_insn.
      IResource *rd = GetRDataResource(insn->GetResource());
      IInsn *rd_insn = new IInsn(rd);
      rd_insn->inputs_ = insn->inputs_;
      rd_insn->outputs_ = insn->outputs_;
      new_insns.push_back(rd_insn);
    }
    st->insns_ = new_insns;
  }
  // TODO: Set rd_insn->depending_insns_.
}

bool TableConverter::IsArrayRead(IInsn *insn) {
  IResource *res = insn->GetResource();
  auto *klass = res->GetClass();
  if (!resource::IsArray(*klass)) {
    return false;
  }
  if (insn->inputs_.size() == 0 &&
      insn->outputs_.size() == 1) {
    return true;
  }
  return false;
}

IResource *TableConverter::GetRDataResource(IResource *a) {
  auto it = array_to_rdata_.find(a);
  IResource *r = nullptr;
  if (it == array_to_rdata_.end()) {
    r = DesignUtil::CreateResource(tab_, resource::kArrayRData);
    tab_->resources_.push_back(r);
    array_to_rdata_[a] = r;
  } else {
    r = it->second;
  }
  return r;
}

}  // namespace

ArraySplitRData::~ArraySplitRData() {
}

Phase *ArraySplitRData::Create() {
  return new ArraySplitRData();
}

bool ArraySplitRData::ApplyForTable(const string &key, ITable *table) {
  TableConverter cv(table);
  cv.Convert();
  return true;
}

}  // namespace opt
}  // namespace iroha
