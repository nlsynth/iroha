#include "writer/html_writer.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "opt/optimizer_log.h"

namespace iroha {
namespace writer {

HtmlWriter::HtmlWriter(const IDesign *design, ostream &os)
    : design_(design), os_(os), opt_log_(design->GetOptimizerLog()) {}

void HtmlWriter::Write() {
  WriteHeader(os_);
  for (auto *mod : design_->modules_) {
    WriteModule(*mod);
  }
  WriteFooter(os_);
}

void HtmlWriter::WriteHeader(ostream &os) {
  os << "<html><head></head><body>\n";
}

void HtmlWriter::WriteFooter(ostream &os) { os << "</body></html>\n"; }

void HtmlWriter::WriteModule(const IModule &mod) {
  os_ << "<div>\n";
  os_ << " module: " << mod.GetName() << "\n";
  for (auto *tab : mod.tables_) {
    WriteTable(*tab);
  }
  os_ << "</div>\n";
}

void HtmlWriter::WriteIntermediateTable(const ITable &tab) { WriteTable(tab); }

void HtmlWriter::WriteTable(const ITable &tab) {
  os_ << "<div>\n"
      << " table: " << tab.GetId() << "<br>\n";
  if (opt_log_ != nullptr) {
    os_ << " " << opt_log_->GetTableAnnotation(&tab) << "\n";
  }
  WriteRegisters(tab);
  WriteResources(tab);
  WriteTableStates(tab);
  os_ << "</div>\n";
}

void HtmlWriter::WriteTableStates(const ITable &tab) {
  os_ << "  <table border=1>\n"
      << "   <thead>\n"
      << "    <td></td>\n";
  for (auto *res : tab.resources_) {
    os_ << "     <td>\n";
    os_ << "     " << res->GetId() << ":" << res->GetClass()->GetName() << "\n";
    os_ << "     </td>\n";
  }
  os_ << "   </thead>\n";

  int color_index = 0;
  int block_index = 0;
  int in_block_index = 0;
  for (auto *st : tab.states_) {
    in_block_index++;
    if (opt_log_ != nullptr) {
      int tmp_block_index = opt_log_->GetStateColorIndex(st);
      if (tmp_block_index != block_index) {
        in_block_index = 0;
        ++color_index;
      }
      block_index = tmp_block_index;
    }
    os_ << "    <tr" << StateRowStyle(color_index, in_block_index) << ">\n";
    os_ << "    <td>\n"
        << "     " << st->GetId() << "\n";
    if (opt_log_ != nullptr) {
      os_ << " " << opt_log_->GetStateAnnotation(st);
    }
    os_ << "    </td>\n";
    map<IResource *, vector<IInsn *> > insns;
    for (IInsn *insn : st->insns_) {
      insns[insn->GetResource()].push_back(insn);
    }
    for (IResource *res : tab.resources_) {
      os_ << "     <td>\n";
      for (IInsn *insn : insns[res]) {
        WriteInsn(*insn);
      }
      os_ << "     </td>\n";
    }
    os_ << "    </tr>\n";
  }
  os_ << "</table>\n";
}

void HtmlWriter::WriteInsn(const IInsn &insn) {
  os_ << "    " << insn.GetId() << ":" << insn.GetOperand();
  if (insn.target_states_.size()) {
    for (auto *st : insn.target_states_) {
      os_ << " " << st->GetId();
    }
  }
  os_ << "(";
  for (auto *reg : insn.inputs_) {
    WriteRegisterId(*reg);
  }
  os_ << ") -&gt (";
  for (auto *reg : insn.outputs_) {
    WriteRegisterId(*reg);
  }
  os_ << ")";
  if (insn.conditions_.size() > 0) {
    os_ << "{";
    for (auto *reg : insn.conditions_) {
      WriteRegisterId(*reg);
    }
    os_ << "}";
  }
  if (opt_log_ != nullptr) {
    os_ << " " << opt_log_->GetInsnAnnotation(&insn);
  }
  os_ << "\n";
}

void HtmlWriter::WriteRegisterId(const IRegister &reg) {
  os_ << " " << reg.GetId();
  if (reg.IsStateLocal()) {
    os_ << "*";
  }
  if (reg.IsConst()) {
    os_ << "[" << reg.GetInitialValue().GetValue0() << "]";
  }
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
  auto *params = reg.GetParams();
  if (params != nullptr) {
    WriteResourceParams(*params);
  }
  if (opt_log_ != nullptr) {
    os_ << " " << opt_log_->GetRegAnnotation(&reg);
  }
  os_ << "\n";
}

void HtmlWriter::WriteValue(const Numeric &val) {
  os_ << " " << val.GetValue0() << " (" << val.type_.GetWidth() << ")";
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

void HtmlWriter::WriteResourceParams(const ResourceParams &params) {
  vector<string> keys = params.GetParamKeys();
  for (string &key : keys) {
    os_ << " " << key << "=";
    vector<string> values = params.GetValues(key);
    os_ << Util::Join(values, ",");
  }
}

string HtmlWriter::StateRowStyle(int block_index, int in_block_index) {
  string color = "#";
  if (block_index % 2) {
    if (in_block_index % 2) {
      color += "88ffff";
    } else {
      color += "ccffff";
    }
  } else {
    if (in_block_index % 2) {
      color += "ff88ff";
    } else {
      color += "ffccff";
    }
  }
  return " style=\"background-color: " + color + "\"";
}

}  // namespace writer
}  // namespace iroha
