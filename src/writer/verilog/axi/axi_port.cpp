#include "writer/verilog/axi/axi_port.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "writer/module_template.h"
#include "writer/verilog/axi/controller.h"
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

AxiPort::AxiPort(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void AxiPort::BuildResource() {
  CHECK(tab_.GetITable() == res_.GetTable());
  string s = BuildPort();
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

void AxiPort::BuildInsn(IInsn *insn, State *st) {
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

string AxiPort::ControllerName(const IResource &res, bool reset_polarity) {
  const IResource *mem_res = res.GetParentResource();
  IArray *array = mem_res->GetArray();
  int addr_width = array->GetAddressWidth();
  string s = "axi_controller_a" + Util::Itoa(addr_width);
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

void AxiPort::WriteController(const IResource &res,
			      bool reset_polarity,
			      ostream &os) {
  Controller c(res, reset_polarity);
  c.Write(os);
}

void AxiPort::BuildInstance(const string &s) {
  bool reset_polarity = tab_.GetModule()->GetResetPolarity();
  tab_.GetEmbeddedModules()->RequestAxiController(&res_, reset_polarity);
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = ControllerName(res_, reset_polarity);
  const string &clk = tab_.GetPorts()->GetClk();
  const string &rst = tab_.GetPorts()->GetReset();
  const IResource *mem = res_.GetParentResource();
  es << "  " << name << " inst_" << name
     << "(.clk("<< clk << "), "
     << ".addr(" << AddrPort() << "), "
     << ".wen(" << WenPort() << "), "
     << ".req(" << ReqPort() << "), "
     << ".ack(" << AckPort() << "), "
     << "." << Controller::ResetName(reset_polarity) << "(" << rst << "), "
     << ".sram_addr(" << SharedMemory::MemoryAddrPin(*mem, 1, nullptr) << "), "
     << ".sram_wdata(" << SharedMemory::MemoryWdataPin(*mem, 1, nullptr)
     << "), "
     << ".sram_rdata(" << SharedMemory::MemoryRdataPin(*mem, 1) << "), "
     << ".sram_wen(" << SharedMemory::MemoryWenPin(*mem, 1, nullptr) << ") "
     << s
     << ");\n";
}

string AxiPort::BuildPort() {
  bool r, w;
  GetReadWrite(res_, &r, &w);
  Module *mod = tab_.GetModule();
  string s;
  Controller::AddPorts(mod, r, w, &s);
  for (mod = mod->GetParentModule(); mod != nullptr;
       mod = mod->GetParentModule()) {
    Controller::AddPorts(mod, r, w, nullptr);
  }
  return s;
}

void AxiPort::GetReadWrite(const IResource &res, bool *r, bool *w) {
  vector<IInsn *> insns = DesignUtil::GetInsnsByResource(&res);
  *r = false;
  *w = false;
  for (IInsn *insn : insns) {
    if (insn->GetOperand() == "read") {
      *r = true;
    } else if (insn->GetOperand() == "write") {
      *w = true;
    } else {
      CHECK(false);
    }
  }
}

string AxiPort::PortSuffix() {
  const ITable *tab = res_.GetTable();
  return Util::Itoa(tab->GetId());
}

string AxiPort::AddrPort() {
  return "axi_addr" + PortSuffix();
}

string AxiPort::WenPort() {
  return "axi_wen" + PortSuffix();
}

string AxiPort::ReqPort() {
  return "axi_req" + PortSuffix();
}

string AxiPort::AckPort() {
  return "axi_ack" + PortSuffix();
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
