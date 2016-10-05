#include "writer/cxx/table.h"

#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "writer/cxx/class_writer.h"
#include "writer/cxx/state.h"

#include <sstream>

using std::ostringstream;

namespace iroha {
namespace writer {
namespace cxx {

Table::Table(const ITable *i_tab) : i_tab_(i_tab) {
  name_ = i_tab->GetModule()->GetName()
    + "_" + Util::Itoa(i_tab->GetId());
  class_writer_.reset(new ClassWriter(name_, "TableBase"));
}

Table::~Table() {
  STLDeleteValues(&states_);
}

void Table::Build() {
  BuildConstructor();
  class_writer_->AddVariable(GetStateVariableName(), "int");
  for (auto *i_st : i_tab_->states_) {
    State *st = new State(i_st, this);
    states_.push_back(st);
    st->Build();
  }
  BuildDispatcher();
  BuildRegisters();
}

void Table::Write(ostream &os) {
  class_writer_->Write(os);
}

ClassWriter *Table::GetClassWriter() {
  return class_writer_.get();
}

void Table::BuildConstructor() {
  const string &name = class_writer_->GetName();
  ClassMember *cm = class_writer_->AddMethod(name, "");
  ostringstream os;
  os << "    st_ = " << i_tab_->GetInitialState()->GetId() << ";\n";
  for (auto *reg : i_tab_->registers_) {
    if (!reg->IsConst() && reg->HasInitialValue()) {
      os << "    " << reg->GetName() << " = "
	 << reg->GetInitialValue().value_ << ";\n";
    }
  }
  cm->body_ = os.str();
}

void Table::BuildDispatcher() {
  ClassMember *cm = class_writer_->AddMethod("dispatch", "virtual void");
  ostringstream os;
  os << "    switch (st_) {\n";
  for (State *st : states_) {
    os << "    case " << st->GetIState()->GetId() << ":\n";
    os << "      " << st->GetMethodName() << "();\n";
    os << "      break;\n";
  }
  os << "    }\n";
  cm->body_ = os.str();
}

void Table::BuildRegisters() {
  for (auto *reg : i_tab_->registers_) {
    if (reg->IsConst()) {
      continue;
    }
    class_writer_->AddVariable(reg->GetName(), "int");
  }
}

string Table::GetTableName() {
  return name_;
}

string Table::GetStateVariableName() {
  return "st_";
}

}  // namespace cxx
}  // namespace writer
}  // namespace iroha
