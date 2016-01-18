#include "writer/html_writer.h"

#include "iroha/i_design.h"
#include "opt/debug_annotation.h"

namespace iroha {

HtmlWriter::HtmlWriter(const IDesign *design, ostream &os)
  : design_(design), os_(os), annotation_(design->GetDebugAnnotation()) {
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

void HtmlWriter::WriteIntermediateTable(const ITable &tab) {
  WriteTable(tab);
}

void HtmlWriter::WriteTable(const ITable &tab) {
  os_ << "<div>\n"
      << " table: " << tab.GetId() << "<br>\n";
  if (annotation_) {
    os_ << " " << annotation_->GetTableAnnotation(&tab) << "\n";
  }
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
  os_ << "<li> state:" << st.GetId();
  if (annotation_) {
    os_ << " " << annotation_->GetStateAnnotation(&st);
  }
  os_ << "\n"
      << " <ul>\n";
  for (auto *insn : st.insns_) {
    WriteInsn(*insn);
  }
  os_ << " </ul>\n</li>\n";
}

void HtmlWriter::WriteInsn(const IInsn &insn){
  auto *res = insn.GetResource();
  os_ << "   <li>insn: " << insn.GetId()
      << " " << res->GetId() << ":" << res->GetClass()->GetName();
  if (insn.target_states_.size()) {
    os_ << " transition:";
    for (auto *st : insn.target_states_) {
      os_ << " " << st->GetId();
    }
  }
  os_ << "(";
  for (auto *reg : insn.inputs_) {
    os_ << " " << reg->GetId();
  }
  os_ << ") -&gt (";
  for (auto *reg : insn.outputs_) {
    os_ << " " << reg->GetId();
  }
  os_ << ")\n";
}

void HtmlWriter::WriteRegisters(const ITable &tab) {
  os_ << "  <div>registers:\n"
      << "   <ul>\n";
  for (auto *reg : tab.registers_) {
    WriteRegister(*reg);
  }
  os_ << "   </ul>\n"
      << "  </div>\n";
}

void HtmlWriter::WriteRegister(const IRegister &reg) {
  os_ << "    <li>r_" << reg.GetId() << ":" << reg.GetName();
  if (reg.IsConst()) {
    os_ << " const";
  }
  if (reg.IsStateLocal()) {
    os_ << " wire";
  }
  if (reg.HasInitialValue()) {
    WriteValue(reg.GetInitialValue());
  } else {
    os_ << " (" << reg.value_type_.GetWidth() << ")";
  }
  os_ << "\n";
}

void HtmlWriter::WriteValue(const IValue &val) {
  os_ << " " << val.value_ << " (" << val.type_.GetWidth() << ")";
}

void HtmlWriter::WriteResources(const ITable &tab) {
  os_ << "  <div>resources:\n"
      << "   <ul>\n";
  for (auto *res : tab.resources_) {
    WriteResource(*res);
  }
  os_ << "   </ul>\n"
      << "  </div>\n";
}

void HtmlWriter::WriteResource(const IResource &res) {
  os_ << "    <li>" << res.GetId() << ":" << res.GetClass()->GetName() << "\n";
}

}  // namespace iroha
