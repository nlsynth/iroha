#include "writer/verilog/axi/axi_port.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "writer/module_template.h"
#include "writer/verilog/axi/controller.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
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
  string s = BuildPort();
  BuildInstance(s);
}

void AxiPort::BuildInsn(IInsn *insn, State *st) {
}

string AxiPort::ControllerName(const IResource &res, bool reset_polarity) {
  return "axi_controller";
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
  const IResource *mem = res_.GetSharedRegister();
  es << "  " << name << " inst_" << name
     << "(.clk("<< clk << "), "
     << "." << Controller::ResetName(reset_polarity) << "(" << rst << "), "
     << ".addr(" << SharedMemory::MemoryAddrPin(*mem, 1, nullptr) << "), "
     << ".wdata(" << SharedMemory::MemoryWdataPin(*mem, 1, nullptr) << "), "
     << ".rdata(" << SharedMemory::MemoryRdataPin(*mem, 1) << "), "
     << ".wen(" << SharedMemory::MemoryWenPin(*mem, 1, nullptr) << ")"
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
    }
    if (insn->GetOperand() == "write") {
      *w = true;
    }
  }
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
