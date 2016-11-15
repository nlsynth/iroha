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
			     EmbeddedModules *embed, Names *names,
			     ModuleTemplate *tmpl)
  : Table(table, ports, mod, embed, names, tmpl) {
}

DataFlowTable::~DataFlowTable() {
}

void DataFlowTable::Build() {
  Table::Build();
  map<const IState *, DataFlowState *> is_to_dfs;
  for (auto *ds : df_states_) {
    is_to_dfs[ds->GetIState()] = ds;
  }
  // Mapping from target state to source states.
  map<DataFlowState *, vector<DataFlowStateTransition> > trs;
  for (auto *ds : df_states_) {
    vector<DataFlowStateTransition> outgoing = ds->GetTransitions();
    for (auto &tr : outgoing) {
      tr.to = is_to_dfs[tr.to_raw];
      trs[tr.to].push_back(tr);
    }
  }
  for (auto *ds : df_states_) {
    ds->BuildIncomingTransitions(trs[ds]);
  }
}

void DataFlowTable::BuildStates() {
  for (auto *i_state : i_table_->states_) {
    DataFlowState *st = new DataFlowState(i_state, this, names_);
    st->Build();
    states_.push_back(st);
    df_states_.push_back(st);
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
  os << StateOutputSectionContents();
  for (auto *state : states_) {
    state->Write(os);
  }
}

string DataFlowTable::GetStateCondition(const IState *st) const {
  return DataFlowState::StateVariable(st);
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
