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
  // Wire to kick this task.
  static string TaskEnableWire(const ITable &tab);

  static string TaskEnablePin(const ITable &tab, const IResource *caller);
  static string TaskAckPin(const ITable &tab, const IResource *caller);
  static string TaskArgPin(const ITable &tab, int nth,
			   const IResource *caller);

  static const int kTaskEntryStateId;

private:
  void BuildWireSet();
  static string TaskArgName(int nth);
  static string TaskPinPrefix(const ITable &tab, const IResource *caller);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_task_h_
