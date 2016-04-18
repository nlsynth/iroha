// -*- C++ -*-
#ifndef _writer_verilog_task_h_
#define _writer_verilog_task_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

class Task {
public:
  Task(Table *table, IInsn *insn);

  void BuildSubModuleTaskResource(const IResource &res);
  void BuildSiblingTaskResource(const IResource &res);

  static Task *MayCreateTask(Table *table);
  static string TaskEnablePin(const ITable &tab);
  static string SubModuleTaskControlPinPrefix(const IResource &res);

  static const int kTaskEntryStateId;

private:
  Table *table_;
  IInsn *task_entry_insn_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_task_h_
