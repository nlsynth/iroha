// -*- C++ -*-
#ifndef _writer_html_writer_h_
#define _writer_html_writer_h_

#include "iroha/common.h"

namespace iroha {

namespace opt {
class OptimizerLog;
}  // namespace opt

namespace writer {

class HtmlWriter {
 public:
  HtmlWriter(const IDesign *design, ostream &os);

  void Write();
  void WriteIntermediateTable(const ITable &tab);
  static void WriteHeader(ostream &os);
  static void WriteFooter(ostream &os);

 private:
  void WriteModule(const IModule &mod);
  void WriteTable(const ITable &tab);
  void WriteTableStates(const ITable &tab);
  void WriteInsn(const IInsn &insn);
  void WriteRegisters(const ITable &tab);
  void WriteRegister(const IRegister &reg);
  void WriteRegisterId(const IRegister &reg);
  void WriteResources(const ITable &tab);
  void WriteResource(const IResource &res);
  void WriteValue(const Numeric &val);
  void WriteResourceParams(const ResourceParams &params);
  string StateRowStyle(int block_index, int in_block_index);

  const IDesign *design_;
  ostream &os_;
  const opt::OptimizerLog *opt_log_;
};

}  // namespace writer
}  // namespace iroha

#endif  // _writer_html_writer_h_
