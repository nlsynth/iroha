// -*- C++ -*-
#ifndef _writer_writer_h_
#define _writer_writer_h_

#include "iroha/writer_api.h"

namespace iroha {
namespace writer {

class Writer : public WriterAPI {
public:
  Writer(const IDesign *design);

  virtual bool Write(const string &fn) override;
  virtual bool SetLanguage(const string &lang) override;
  virtual void OutputShellModule(bool b, bool self_clock, bool vcd) override;
  virtual void SetOutputConfig(const string &root,
			       const string &marker,
			       bool debug) override;

  // For debug dump.
  static void WriteDumpHeader(ostream &os);
  static void WriteDumpFooter(ostream &os);
  static void DumpTable(const ITable *table, ostream &os);

private:
  static string ShellModuleName(const string &fn);

  const IDesign *design_;
  string language_;
  bool output_shell_module_;
  bool output_self_clock_;
  bool output_vcd_;
  string output_marker_;
  bool debug_;
  string root_dir_;
};

}  // namespace writer
}  // namespace iroha

#endif  // _writer_writer_h_
