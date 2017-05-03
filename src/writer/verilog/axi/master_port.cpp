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
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"
#include "writer/verilog/shared_memory.h"

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
  BuildInstance(s);

  ostream &os = tmpl_->GetStream(kResourceSection);
  os << "  reg [31:0] " << AddrPort() << ";\n"
     << "  reg " << WenPort() << ";\n"
     << "  reg " << ReqPort() << ";\n"
     << "  wire " << AckPort() << ";\n";

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
    ss << JoinStatesWithSubState(accessors, 0) << ";\n";
  }
  map<IState *, IInsn *> writers;
  CollectResourceCallers("write", &writers);
  ss << "      " << WenPort() << " <= ";
  if (writers.size() == 0) {
    ss << "0;\n";
  } else {
    ss << JoinStatesWithSubState(writers, 0) << ";\n";
  }
}

void MasterPort::BuildInsn(IInsn *insn, State *st) {
    static const char I[] = "          ";
  string insn_st = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  ostream &os = st->StateBodySectionStream();
  os << I << "// AXI access request\n"
     << I << "if (" << insn_st << " == 0) begin\n"
     << I << "  " << AddrPort() << " <= "
     << InsnWriter::RegisterValue(*insn->inputs_[0], tab_.GetNames()) << ";\n"
     << I << "  if (" << AckPort() << ") begin\n"
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
  return s;
}

void MasterPort::WriteController(const IResource &res,
				 bool reset_polarity,
				 ostream &os) {
  MasterController c(res, reset_polarity);
  c.Write(os);
}

void MasterPort::BuildInstance(const string &s) {
  bool reset_polarity = tab_.GetModule()->GetResetPolarity();
  tab_.GetEmbeddedModules()->RequestAxiMasterController(&res_, reset_polarity);
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = ControllerName(res_, reset_polarity);
  const string &clk = tab_.GetPorts()->GetClk();
  const string &rst = tab_.GetPorts()->GetReset();
  const IResource *mem = res_.GetParentResource();
  es << "  " << name << " inst_" << name
     << "(.clk("<< clk << "), "
     << "." << MasterController::ResetName(reset_polarity)
     << "(" << rst << "), "
     << ".addr(" << AddrPort() << "), "
     << ".wen(" << WenPort() << "), "
     << ".req(" << ReqPort() << "), "
     << ".ack(" << AckPort() << "), "
     << ".sram_addr(" << SharedMemory::MemoryAddrPin(*mem, 1, nullptr) << "), "
     << ".sram_wdata(" << SharedMemory::MemoryWdataPin(*mem, 1, nullptr)
     << "), "
     << ".sram_rdata(" << SharedMemory::MemoryRdataPin(*mem, 1) << "), "
     << ".sram_wen(" << SharedMemory::MemoryWenPin(*mem, 1, nullptr) << ") "
     << s
     << ");\n";
}

string MasterPort::BuildPortToExt() {
  bool r, w;
  GetReadWrite(res_, &r, &w);
  Module *mod = tab_.GetModule();
  string s;
  MasterController::AddPorts(mod, r, w, &s);
  for (mod = mod->GetParentModule(); mod != nullptr;
       mod = mod->GetParentModule()) {
    MasterController::AddPorts(mod, r, w, nullptr);
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

string MasterPort::PortSuffix() {
  const ITable *tab = res_.GetTable();
  return Util::Itoa(tab->GetId());
}

string MasterPort::AddrPort() {
  return "axi_addr" + PortSuffix();
}

string MasterPort::WenPort() {
  return "axi_wen" + PortSuffix();
}

string MasterPort::ReqPort() {
  return "axi_req" + PortSuffix();
}

string MasterPort::AckPort() {
  return "axi_ack" + PortSuffix();
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
