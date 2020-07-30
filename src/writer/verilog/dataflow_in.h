// -*- C++ -*-
#ifndef _writer_verilog_dataflow_in_h_
#define _writer_verilog_dataflow_in_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class DataFlowIn : public Resource {
public:
  DataFlowIn(const IResource &res, const Table &table);
  virtual ~DataFlowIn() {};

  virtual void BuildResource();
  virtual void BuildInsn(IInsn *insn, State *st);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_dataflow_in_h_
