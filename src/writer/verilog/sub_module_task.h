// -*- C++ -*-
#ifndef _writer_verilog_sub_module_task_h_
#define _writer_verilog_sub_module_task_h_

#include "writer/verilog/task.h"

namespace iroha {
namespace writer {
namespace verilog {

class SubModuleTask : public Task {
public:
  SubModuleTask(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;
  virtual string ReadySignal(IInsn *insn) override;

  static string SubModuleTaskCallerPinPrefix(const IResource &res);
  static void BuildChildTaskWire(const TaskCallInfo &ti,
				 ostream &os);
  static void BuildPorts(const TaskCallInfo &ti, Ports *ports);
  static string PortNamePrefix(const ITable &callee_tab);

private:
  void BuildSubModuleTask();
  void BuildSubModuleTaskCallInsn(IInsn *insn, State *st);
  void BuildSubModuleTaskCall();
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_sub_module_task_h_
