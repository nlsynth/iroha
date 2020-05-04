// -*- C++ -*-
#ifndef _writer_verilog_dataflow_table_h_
#define _writer_verilog_dataflow_table_h_

#include "writer/verilog/common.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

class DataFlowTable : public Table {
public:
  DataFlowTable(ITable *table, PortSet *ports, Module *mod,
		EmbeddedModules *embed, Names *names,
		ModuleTemplate *tmpl);

  virtual ~DataFlowTable();

  virtual void Build();
  virtual void Write(ostream &os);

  virtual string GetStateCondition(const IState *st) const;
  virtual const DataFlowTable *GetDataFlowTable() const;

  string BlockingCondition() const;
  bool CanBlock() const;

private:
  virtual void BuildStates();
  virtual void BuildStateDecl();
  virtual void WriteReset(ostream &os);
  virtual void WriteBody(ostream &os);

  bool ScanMultiCycle();
  void BuildBlockingCondition(ostream &os) const;

  // Same vector as states_.
  vector<DataFlowState *> df_states_;
  bool has_multi_cycle_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_dataflow_table_h_
