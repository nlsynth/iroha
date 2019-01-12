// -*- C++ -*-
#ifndef _writer_verilog_task_h_
#define _writer_verilog_task_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class Task : public Resource {
public:
  Task(const IResource &res, const Table &table);
  virtual void BuildResource();
  virtual void BuildInsn(IInsn *insn, State *st);

  static bool IsTask(const Table &table);
  static string TaskEnablePin(const ITable &tab, const ITable *caller);
  static string TaskAckPin(const ITable &tab, const ITable *caller);
  static string TaskArgPin(const ITable &tab, int nth, bool output,
			   const ITable *caller);

  static const int kTaskEntryStateId;

private:
  static string TaskPinPrefix(const ITable &tab, const ITable *caller);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_task_h_
