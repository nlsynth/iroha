#include "writer/verilog/axi/master_port.h"

#include "design/design_util.h"
#include "iroha/insn_operands.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "writer/module_template.h"
#include "writer/verilog/axi/master_controller.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/shared_memory.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

MasterPort::MasterPort(const IResource &res, const Table &table)
  : AxiPort(res, table) {
}

void MasterPort::BuildResource() {
  CHECK(tab_.GetITable() == res_.GetTable());
  string s = BuildPortToExt();
  BuildControllerInstance(s);
  if (!IsExclusiveAccessor()) {
    SharedMemory::BuildMemoryAccessorResource(*this, true, false,
					      res_.GetParentResource());
  }

  ostream &os = tmpl_->GetStream(kResourceSection);
  os << "  reg [31:0] " << AddrPort() << ";\n"
     << "  reg " << WenPort() << ";\n"
     << "  reg " << ReqPort() << ";\n"
     << "  wire " << AckPort() << ";\n";
  PortConfig cfg = AxiPort::GetPortConfig(res_);
  int aw = cfg.sram_addr_width - 1;
  os << "  reg [" << aw << ":0] " << LenPort() << ";\n"
     << "  reg [" << aw << ":0] " << StartPort() << ";\n";

  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << AddrPort() << " <= 0;\n"
     << "      " << WenPort() << " <= 0;\n"
     << "      " << ReqPort() << " <= 0;\n";

  map<IState *, IInsn *> accessors;
  CollectResourceCallers("*", &accessors);
  ostream &ss = tab_.StateOutputSectionStream();
  ss << "      " << ReqPort() << " <= ";
  if (accessors.size() == 0) {
    ss << "0;\n";
  } else {
    ss << JoinStatesWithSubState(accessors, 0) << " && !"
       << AckPort() <<";\n";
  }
  map<IState *, IInsn *> writers;
  CollectResourceCallers("write", &writers);
  ss << "      " << WenPort() << " <= ";
  if (writers.size() == 0) {
    ss << "0;\n";
  } else {
    ss << JoinStatesWithSubState(writers, 0)  << " && !"
       << AckPort() <<";\n";
  }
}

void MasterPort::BuildInsn(IInsn *insn, State *st) {
  static const char I[] = "          ";
  string insn_st = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  ostream &os = st->StateBodySectionStream();
  os << I << "// AXI access request\n"
     << I << "if (" << insn_st << " == 0) begin\n"
     << I << "  " << AddrPort() << " <= "
     << InsnWriter::RegisterValue(*insn->inputs_[0], tab_.GetNames()) << ";\n";
  // Length.
  os << I << "  " << LenPort() << " <= ";
  if (insn->inputs_.size() >= 2) {
    os << InsnWriter::RegisterValue(*insn->inputs_[1], tab_.GetNames());
  } else {
    os << "~0";
  }
  os << ";\n";
  // Start.
  os << I << "  " << StartPort() << " <= ";
  if (insn->inputs_.size() >= 3) {
    os << InsnWriter::RegisterValue(*insn->inputs_[2], tab_.GetNames());
  } else {
    os << "0";
  }
  os << ";\n";
  os << I << "  if (" << AckPort() << ") begin\n"
     << I << "    " << insn_st << " <= 3;\n"
     << I << "  end\n"
     << I << "end\n";
}

string MasterPort::ControllerName(const IResource &res, bool reset_polarity) {
  const IResource *mem_res = res.GetParentResource();
  IArray *array = mem_res->GetArray();
  int addr_width = array->GetAddressWidth();
  string s = "axi_master_controller_a" + Util::Itoa(addr_width);
  bool r, w;
  GetReadWrite(res, &r, &w);
  if (r) {
    s += "r";
  }
  if (w) {
    s += "w";
  }
  s += "d" + Util::Itoa(array->GetDataType().GetWidth());
  return s;
}

void MasterPort::WriteController(const IResource &res,
				 bool reset_polarity,
				 ostream &os) {
  MasterController c(res, reset_polarity);
  c.Write(os);
}

void MasterPort::BuildControllerInstance(const string &wires) {
  tab_.GetEmbeddedModules()->RequestAxiMasterController(&res_, reset_polarity_);
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = ControllerName(res_, reset_polarity_);
  es << "  " << name << " inst_" << PortSuffix()
     << "_" << name << "(";
  OutputSRAMConnection(es);
  es << ", .addr(" << AddrPort() << "), "
     << ".wen(" << WenPort() << "), "
     << ".req(" << ReqPort() << "), "
     << ".len(" << LenPort() << "), "
     << ".start(" << StartPort() << "), "
     << ".ack(" << AckPort() << ") "
     << wires
     << ");\n";
}

string MasterPort::BuildPortToExt() {
  Module *mod = tab_.GetModule();
  string s;
  PortConfig cfg = AxiPort::GetPortConfig(res_);
  MasterController::AddPorts(cfg, mod, true, true, &s);
  for (mod = mod->GetParentModule(); mod != nullptr;
       mod = mod->GetParentModule()) {
    MasterController::AddPorts(cfg, mod, true, true, nullptr);
  }
  return s;
}

void MasterPort::GetReadWrite(const IResource &res, bool *r, bool *w) {
  vector<IInsn *> insns = DesignUtil::GetInsnsByResource(&res);
  *r = false;
  *w = false;
  for (IInsn *insn : insns) {
    if (insn->GetOperand() == operand::kRead) {
      *r = true;
    } else if (insn->GetOperand() == operand::kWrite) {
      *w = true;
    } else {
      CHECK(false);
    }
  }
}

string MasterPort::LenPort() {
  return "axi_len" + PortSuffix();
}

string MasterPort::StartPort() {
  return "axi_start" + PortSuffix();
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
