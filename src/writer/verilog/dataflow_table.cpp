#include "writer/verilog/dataflow_table.h"

#include "design/resource_attr.h"
#include "iroha/i_design.h"
#include "writer/module_template.h"
#include "writer/verilog/dataflow_state.h"
#include "writer/verilog/insn_writer.h"
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
  has_multi_cycle_ = ScanMultiCycle();
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
  sd << "  // stage tokens\n";
  ostream &is = InitialValueSectionStream();
  for (auto *st : i_table_->states_) {
    string s = DataFlowState::StateVariable(st);
    sd << "  reg " << s << ";\n";
    is << "      " << s << " <= 0;\n";
    if (has_multi_cycle_) {
      string w = DataFlowState::StateWaitVariable(st);
      sd << "  reg " << w << ";\n";
      is << "      " << w << " <= 0;\n";
    }
  }
  if (has_multi_cycle_) {
    BuildBlockingCondition(sd);
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

string DataFlowTable::BlockingCondition() const {
  return "block_" + Util::Itoa(i_table_->GetId());
}

void DataFlowTable::BuildBlockingCondition(ostream &os) const {
  os << "  wire " << BlockingCondition() << ";\n";
  os << "  assign " << BlockingCondition() << " = ";
  vector<string> conds;
  for (auto *state : states_) {
    if (!state->IsCompoundCycle()) {
      continue;
    }
    const IState *ist = state->GetIState();
    vector<string> subs;
    for (IInsn *insn : ist->insns_) {
      if (ResourceAttr::IsMultiCycleInsn(insn)) {
	string d = InsnWriter::MultiCycleStateName(*insn->GetResource());
	d = d + " == 3";
	subs.push_back(d);
      }
    }
    string c = "(" + DataFlowState::StateVariable(ist) + " || (" +
      DataFlowState::StateWaitVariable(ist) +
      " && !(" + Util::Join(subs, " && ") + ")))";
    conds.push_back(c);
  }
  os << Util::Join(conds, " || ") << ";\n";
}

bool DataFlowTable::CanBlock() const {
  return has_multi_cycle_;
}

bool DataFlowTable::ScanMultiCycle() {
  // Uses raw iroha objects instead of State, since this might be called
  // before Table::Build().
  for (auto *i_state : i_table_->states_) {
    if (ResourceAttr::NumMultiCycleInsn(i_state) > 0) {
      return true;
    }
  }
  return false;
}

const DataFlowTable *DataFlowTable::GetDataFlowTable() const {
  return this;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
