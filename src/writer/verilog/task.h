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

  static const int kTaskEntryStateId;

private:
  void BuildTaskResource();
  void BuildTaskCallResource();
  void BuildCallWire(IResource *caller);
  void BuildTaskCallInsn(IInsn *insn, State *st);
  void AddDownwardPort(IModule *mod, IResource *caller);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_task_h_
