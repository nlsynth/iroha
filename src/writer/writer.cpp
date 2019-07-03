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
  : design_(design), output_shell_module_(false), output_self_clock_(false),
    output_vcd_(false), debug_(false) {
}

bool Writer::Write(const string &fn) {
  ostream *os;
  unique_ptr<ostream> os_deleter;
  if (fn.empty()) {
    os = &cout;
  } else {
    string fn_path = fn;
    if (!root_dir_.empty()) {
      fn_path = root_dir_ + "/" + fn_path;
    }
    os = new ofstream(fn_path);
    if (!(*os)) {
      cerr << "Failed to open [" << fn_path << "]\n";
      return false;
    }
    os_deleter.reset(os);
    if (!output_marker_.empty()) {
      cout << output_marker_ << fn << "\n";
    }
  }
  string shell;
  if (output_shell_module_) {
    shell = ShellModuleName(fn);
  }
  bool res = true;
  if (language_ == "verilog") {
    Connection conn(design_);
    conn.Build();
    verilog::VerilogWriter writer(design_, conn, debug_, *os);
    if (!shell.empty()) {
      writer.SetShellModuleName(shell, output_self_clock_, output_vcd_);
    }
    res = writer.Write();
  } else if (language_ == "dot") {
    // TODO: Implement this.
  } else if (language_ == "html") {
    HtmlWriter writer(design_, *os);
    writer.Write();
  } else {
    ExpWriter writer(design_, *os);
    writer.Write();
  }
  if (!(*os)) {
    cerr << "Failed to write [" << fn << "]\n";
    res = false;
  }
  return res;
}

bool Writer::SetLanguage(const string &lang) {
  language_ = lang;
  return true;
}

void Writer::OutputShellModule(bool b, bool self_clock, bool vcd) {
  output_shell_module_ = b;
  output_self_clock_ = self_clock;
  output_vcd_ = vcd;
}

void Writer::SetOutputConfig(const string &root, const string &marker,
			     bool debug) {
  root_dir_ = root;
  output_marker_ = marker;
  debug_ = debug;
}

void Writer::WriteDumpHeader(ostream &os) {
  HtmlWriter::WriteHeader(os);
}

void Writer::WriteDumpFooter(ostream &os) {
  HtmlWriter::WriteFooter(os);
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
