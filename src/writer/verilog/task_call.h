// -*- C++ -*-
#ifndef _writer_verilog_task_call_h_
#define _writer_verilog_task_call_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class TaskCall : public Resource {
public:
  TaskCall(const IResource &res, const Table &table);
  virtual void BuildResource();
  virtual void BuildInsn(IInsn *insn, State *st);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_task_call_h_
