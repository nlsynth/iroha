#include "writer/verilog/shared_reg.h"

#include "iroha/i_design.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

SharedReg::SharedReg(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void SharedReg::BuildResource() {
  vector<pair<IState *, IInsn *>> writers;
  for (IState *st : res_.GetTable()->states_) {
    for (IInsn *insn : st->insns_) {
      if (insn->GetResource() == &res_) {
	if (insn->inputs_.size() > 0) {
	  writers.push_back(make_pair(st, insn));
	}
      }
    }
  }
  IRegister *foreign_reg = res_.GetForeignRegister();
  string res_name = RegPrefix(*tab_.GetITable(), *foreign_reg);
  ostream &rs = tmpl_->GetStream(kResourceSection);
  rs << "  // " << res_name << "\n";
  rs << "  wire " << res_name << "_w;\n";
  rs << "  wire " << Table::WidthSpec(foreign_reg) << " "
     << res_name << "_wdata;\n";
  if (writers.size() == 0) {
    rs << "  assign " << res_name << "_w = 0;\n";
    rs << "  assign " << res_name << "_wdata = 0;\n";
    return;
  }
  vector<IState *> sts;
  for (auto &w : writers) {
    sts.push_back(w.first);
  }
  rs << "  assign " << res_name << "_w = ";
  rs << JoinStates(sts);
  rs << ";\n";

  string d;
  for (auto &w : writers) {
    IInsn *insn = w.second;
    if (d.empty()) {
      d = InsnWriter::RegisterName(*insn->inputs_[0]);
    } else {
      IState *st = w.first;
      string t;
      t = "(" + tab_.StateVariable() + " == " + Util::Itoa(st->GetId()) + ") ? ";
      t += InsnWriter::RegisterName(*insn->inputs_[0]);
      t += " : (" + d + ")";
      d = t;
    }
  }
  rs << "  assign " << res_name << "_wdata = " << d << ";\n";
}

void SharedReg::BuildInsn(IInsn *insn, State *st) {
  if (insn->outputs_.size() == 0) {
    return;
  }
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
     << " = "
     << InsnWriter::RegisterName(*(res_.GetForeignRegister()))
     << ";\n";
}

string SharedReg::RegPrefix(const ITable &writer, const IRegister &reg) {
  return "shared_reg_" + Util::Itoa(writer.GetId()) + "_" + Util::Itoa(reg.GetTable()->GetId()) + "_" + Util::Itoa(reg.GetId());
}

void SharedReg::BuildSharedRegisters(const Table &tab) {
  Module *mod = tab.GetModule();
  const IModule *i_mod = mod->GetIModule();
  ITable *i_tab = tab.GetITable();
  map<IRegister *, vector<ITable *>> writers;
  // Collects tables which write to registers in this table.
  for (auto *other_tab : i_mod->tables_) {
    if (other_tab->GetId() == i_tab->GetId()) {
      continue;
    }
    for (auto *res : other_tab->resources_) {
      IRegister *reg = res->GetForeignRegister();
      if (reg == nullptr || reg->GetTable() != i_tab) {
	continue;
      }
      writers[reg].push_back(other_tab);
    }
  }

  ostream &os = tab.StateOutputSectionStream();
  for (auto &w : writers) {
    IRegister *reg = w.first;
    bool is_first = true;
    for (auto *t : w.second) {
      os << "      ";
      if (!is_first) {
	os << "else ";
      }
      string s = SharedReg::RegPrefix(*t, *reg);
      os << "if (" << s << "_w) begin\n";
      os << "        " << InsnWriter::RegisterName(*reg) << " <= ";
      os << s << "_wdata;\n";
      os << "      end\n";
      is_first = false;
    }
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
