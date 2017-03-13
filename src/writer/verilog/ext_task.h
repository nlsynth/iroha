// -*- C++ -*-
#ifndef _writer_verilog_ext_task_h_
#define _writer_verilog_ext_task_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class ExtTask : public Resource {
public:
  ExtTask(const IResource &res, const Table &table);

  static bool IsExtTask(const Table &table);
  static string TaskReadyPin(const Table &table);
  static string ReqValidPin(const IResource &res);
  static string ReqReadyPin(const IResource &res);
  static string BusyPin(const IResource &res);
  static string ResValidPin(const IResource &res);
  static string ResReadyPin(const IResource &res);
  static string ArgPin(const IResource &res, int nth);
  // This one takes ext-task-done resource.
  static string DataPin(const IResource &res, int nth);

  virtual void BuildResource();
  virtual void BuildInsn(IInsn *insn, State *st);

private:
  void BuildExtTask();
  void BuildPorts();
  string ArgCaptureReg(int nth);
  void AddPort(const string &port, bool is_output, int width);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_ext_task_h_
