#include "writer/writer.h"

#include "iroha/iroha.h"
#include "writer/connection.h"
#include "writer/exp_writer.h"
#include "writer/html_writer.h"
#include "writer/verilog/verilog_writer.h"

#include <fstream>

namespace iroha {
namespace writer {

Writer::Writer(const IDesign *design)
  : design_(design), output_shell_module_(false) {
}

bool Writer::Write(const string &fn) {
  ostream *os;
  unique_ptr<ostream> os_deleter;
  if (fn.empty()) {
    os = &cout;
  } else {
    os = new ofstream(fn);
    if (os == nullptr) {
      return false;
    }
    os_deleter.reset(os);
  }
  string shell;
  if (output_shell_module_) {
    shell = ShellModuleName(fn);
  }
  if (language_ == "verilog") {
    Connection conn(design_);
    conn.Build();
    verilog::VerilogWriter writer(design_, conn, *os);
    if (!shell.empty()) {
      writer.SetShellModuleName(shell);
    }
    writer.Write();
  } else if (language_ == "html") {
    HtmlWriter writer(design_, *os);
    writer.Write();
  } else {
    ExpWriter writer(design_, *os);
    writer.Write();
  }
  return true;
}

bool Writer::SetLanguage(const string &lang) {
  language_ = lang;
  return true;
}

void Writer::OutputShellModule(bool b) {
  output_shell_module_ = b;
}

void Writer::DumpTable(const ITable *table, ostream &os) {
  HtmlWriter writer(table->GetModule()->GetDesign(), os);
  writer.WriteIntermediateTable(*table);
}

string Writer::ShellModuleName(const string &fn) {
  string mod_name = fn;
  int pos = mod_name.rfind('.');
  if (pos != string::npos) {
    mod_name = string(mod_name.c_str(), pos);
  }
  pos = mod_name.rfind('/');
  if (pos != string::npos) {
    mod_name = string(mod_name.c_str() + pos + 1);
  }
  return mod_name;
}

}  // namespace writer
}  // namespace iroha
