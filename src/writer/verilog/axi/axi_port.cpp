#include "writer/verilog/axi/axi_port.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "writer/verilog/axi/axi_controller.h"
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
  reset_polarity_ = tab_.GetModule()->GetResetPolarity();
}

void AxiPort::OutputSRAMConnection(ostream &os) {
  const string &clk = tab_.GetPorts()->GetClk();
  const string &rst = tab_.GetPorts()->GetReset();
  const IResource *mem = res_.GetParentResource();
  os << ".clk("<< clk << "), "
     << "." << AxiController::ResetName(reset_polarity_)
     << "(" << rst << "), "
     << ".sram_addr(" << SharedMemory::MemoryAddrPin(*mem, 1, nullptr) << "), "
     << ".sram_wdata(" << SharedMemory::MemoryWdataPin(*mem, 1, nullptr)
     << "), "
     << ".sram_rdata(" << SharedMemory::MemoryRdataPin(*mem, 1) << "), "
     << ".sram_wen(" << SharedMemory::MemoryWenPin(*mem, 1, nullptr) << ")";
}

PortConfig AxiPort::GetPortConfig(const IResource &res) {
  PortConfig cfg;
  cfg.addr_width = res.GetParams()->GetAddrWidth();
  const IResource *mem_res = res.GetParentResource();
  IArray *array = mem_res->GetArray();
  cfg.data_width = array->GetDataType().GetWidth();
  return cfg;
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
