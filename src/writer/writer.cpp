#include "writer/writer.h"

#include <fstream>
#include "iroha/iroha.h"
#include "writer/exp_writer.h"
#include "writer/verilog/verilog_writer.h"

namespace iroha {

Writer::Writer(const IDesign *design) : design_(design) {
}

bool Writer::Write(const string &fn) {
  unique_ptr<ostream> os(new ofstream(fn));
  if (os.get() == nullptr) {
    return false;
  }
  if (language_ == "verilog") {
    verilog::VerilogWriter writer(design_, *os);
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

}  // namespace iroha
