#include "writer/verilog/dataflow_table.h"

#include "iroha/i_design.h"
#include "writer/module_template.h"
#include "writer/verilog/dataflow_state.h"
#include "writer/verilog/module.h"
#include "writer/verilog/state.h"

namespace iroha {
namespace writer {
namespace verilog {

DataFlowTable::DataFlowTable(ITable *table, Ports *ports, Module *mod,
			     EmbeddedModules *embed,
			     ModuleTemplate *tmpl)
  : Table(table, ports, mod, embed, tmpl) {
}

DataFlowTable::~DataFlowTable() {
}

void DataFlowTable::Build() {
  Table::Build();
}

void DataFlowTable::BuildStates() {
  for (auto *i_state : i_table_->states_) {
    State *st = new DataFlowState(i_state, this);
    st->Build();
    states_.push_back(st);
  }
}

void DataFlowTable::Write(ostream &os) {
  Table::Write(os);
}

void DataFlowTable::BuildStateDecl() {
  ostream &sd = tmpl_->GetStream(kStateDeclSection);
  ostream &is = InitialValueSectionStream();
  for (auto *st : i_table_->states_) {
    string s = DataFlowState::StateVariable(st);
    sd << "  reg " << s << ";\n";
    is << "      " << s << " <= 0;\n";
  }
}

void DataFlowTable::WriteReset(ostream &os) {
  os << InitialValueSectionContents();
}

void DataFlowTable::WriteBody(ostream &os) {
    for (auto *state : states_) {
      state->Write(os);
    }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
