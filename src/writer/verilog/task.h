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

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;
  virtual string ReadySignal(IInsn *insn) override;

  static bool IsTask(const Table &table);
  static string TaskEnablePin(const ITable &tab, const ITable *caller);
  static string SubModuleTaskControlPinPrefix(const IResource &res);
  static string SiblingTaskReadySignal(const ITable &tab,
				       const ITable *caller);
  static void BuildChildTaskWire(const TaskCallInfo &ti,
				 ostream &os);
  static void BuildPorts(const TaskCallInfo &ti, Ports *ports);

  static const int kTaskEntryStateId;

private:
  void BuildSubModuleTask();
  void BuildSiblingTask();
  void BuildSiblingTaskCall();
  void BuildSubModuleTaskCall();
  void BuildSubModuleTaskCallInsn(IInsn *insn, State *st);
  void BuildSiblingTaskInsn(IInsn *insn, State *st);

  static string ArgSignal(const ITable &tab, int nth, const ITable *caller);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_task_h_
