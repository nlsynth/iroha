#include "writer/html_writer.h"

#include "iroha/i_design.h"

namespace iroha {

HtmlWriter::HtmlWriter(const IDesign *design, ostream &os)
  : design_(design), os_(os) {
}

void HtmlWriter::Write() {
  WriteHeader();
  for (auto *mod : design_->modules_) {
    WriteModule(*mod);
  }
  WriteFooter();
}

void HtmlWriter::WriteHeader() {
  os_ << "<html>\n";
}

void HtmlWriter::WriteFooter() {
  os_ << "</html>\n";
}

void HtmlWriter::WriteModule(const IModule &mod) {
  os_ << "<div>\n";
  os_ << " module: " << mod.GetName() << "\n";
  for (auto *tab : mod.tables_) {
    WriteTable(*tab);
  }
  os_ << "</div>\n";
}

void HtmlWriter::WriteTable(const ITable &tab) {
  os_ << "<div>\n"
      << " table: " << tab.GetId() << "\n";
  WriteRegisters(tab);
  WriteResources(tab);
  os_ << " <ul>\n";
  for (auto *st : tab.states_) {
    WriteState(*st);
  }
  os_ << "</ul>\n"
      << "</div>\n";
}

void HtmlWriter::WriteState(const IState &st) {
  os_ << "<li> state:" << st.GetId() << "\n"
      << " <ul>\n";
  for (auto *insn : st.insns_) {
    WriteInsn(*insn);
  }
  os_ << " </ul>\n</li>\n";
}
  
void HtmlWriter::WriteInsn(const IInsn &insn){
  os_ << "   <li>insn: " << insn.GetId() << "\n";
}
  
void HtmlWriter::WriteRegisters(const ITable &tab) {
}
  
void HtmlWriter::WriteResources(const ITable &tab) {
}
  
}  // namespace iroha
