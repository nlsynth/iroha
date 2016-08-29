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
  static bool IsSiblingTask(const Table &table);
  static string TaskEnablePin(const ITable &tab, const ITable *caller);
  static string SiblingTaskReadySignal(const ITable &tab,
				       const ITable *caller);

  static const int kTaskEntryStateId;

private:
  void BuildSiblingTask();
  void BuildSiblingTaskCall();
  void BuildSiblingTaskInsn(IInsn *insn, State *st);

  static string ArgSignal(const ITable &tab, int nth, const ITable *caller);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_task_h_
