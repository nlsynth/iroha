// -*- C++ -*-
#ifndef _writer_html_writer_h_
#define _writer_html_writer_h_

#include "iroha/common.h"

namespace iroha {

class HtmlWriter {
public:
  HtmlWriter(const IDesign *design, ostream &os);

  void Write();

private:
  void WriteHeader();
  void WriteFooter();
  void WriteModule(const IModule &mod);
  void WriteTable(const ITable &tab);
  void WriteState(const IState &st);
  void WriteInsn(const IInsn &insn);
  void WriteRegisters(const ITable &tab);
  void WriteResources(const ITable &tab);

  const IDesign *design_;
  ostream &os_;
};

}  // namespace iroha

#endif  // _writer_html_writer_h_
