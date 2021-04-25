#include "writer/verilog/self_shell.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/verilog/array.h"
#include "writer/verilog/axi/axi_shell.h"
#include "writer/verilog/embedded_modules.h"
#include "writer/verilog/ext_task.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/port_set.h"
#include "writer/verilog/sram_if.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

SelfShell::SelfShell(const IDesign *design, const PortSet *ports,
                     bool reset_polarity, EmbeddedModules *embedded_modules)
    : design_(design),
      ports_(ports),
      reset_polarity_(reset_polarity),
      embedded_modules_(embedded_modules) {
  for (IModule *mod : design->modules_) {
    ProcessModule(mod);
  }
}

void SelfShell::WriteWireDecl(ostream &os) {
  for (IResource *res : axi_) {
    axi::AxiShell shell(res);
    shell.WriteWireDecl(os);
  }
  for (IResource *res : ext_input_) {
    auto *params = res->GetParams();
    string input_port;
    int width;
    params->GetExtInputPort(&input_port, &width);
    os << "  wire " << Table::WidthSpec(width) << input_port << ";\n";
    os << "  assign " << input_port << " = 0;\n";
  }
  for (IResource *res : ext_output_) {
    auto *params = res->GetParams();
    string output_port;
    int width;
    params->GetExtOutputPort(&output_port, &width);
    os << "  wire " << Table::WidthSpec(width) << output_port << ";\n";
    os << "  always @(" << output_port << ") begin\n"
       << "    $display(\"%t, " << output_port << "=%x\", $time, "
       << output_port << ");\n"
       << "  end\n\n";
  }
  for (IResource *res : ext_task_entry_) {
    string v = ExtTask::ReqValidPin(res);
    os << "  wire " << v << ";\n"
       << "  assign " << v << " = 0;\n";
  }
  for (IResource *res : ext_task_call_) {
    string name = res->GetParams()->GetExtTaskName();
    string req_ready = ExtTask::ReqReadyPin(res);
    string req_valid = ExtTask::ReqValidPin(res);
    os << "  wire " << req_valid << ";\n"
       << "  wire " << req_ready << ";\n"
       << "  assign " << req_ready << " = 1;\n";
    os << "  always @(posedge clk) begin\n"
       << "    if (!rst && " << req_valid << ") begin\n"
       << "    $display(\"task call " << name << "()\");\n"
       << "    end\n"
       << "  end\n\n";
  }
  for (IResource *res : ext_task_wait_) {
    string res_valid = ExtTask::ResValidPin(res->GetParentResource());
    os << "  wire " << res_valid << ";\n"
       << "  assign " << res_valid << " = 1;\n";
    for (int i = 0; i < res->output_types_.size(); ++i) {
      string d = ExtTask::DataPin(res, i);
      os << "  wire " << Table::WidthSpec(res->output_types_[i].GetWidth()) << d
         << ";\n"
         << "  assign " << d << " = 0;\n";
    }
  }
  for (IResource *res : ext_ram_) {
    IArray *arr = res->GetArray();
    int data_width = arr->GetDataType().GetWidth();
    os << "  wire " << Table::WidthSpec(arr->GetAddressWidth())
       << ArrayResource::SigName(*res, "addr") << ";\n";
    os << "  wire " << Table::WidthSpec(data_width)
       << ArrayResource::SigName(*res, "rdata") << ";\n";
    os << "  wire " << Table::WidthSpec(data_width)
       << ArrayResource::SigName(*res, "wdata") << ";\n";
    os << "  wire " << ArrayResource::SigName(*res, "wdata_en") << ";\n";

    // Internal of shell module and external of the design itself.
    InternalSRAM *sram =
        embedded_modules_->RequestInternalSRAM(*arr, 1, reset_polarity_);
    string inst = "_" + Util::Itoa(res->GetTable()->GetModule()->GetId()) +
                  "_" + Util::Itoa(res->GetTable()->GetId()) + "_" +
                  Util::Itoa(res->GetId());
    os << "  " << sram->GetModuleName() << " " << sram->GetModuleName() << inst
       << "("
       << ".clk(" << ports_->GetClk() << ")"
       << ", ." << sram->GetResetPinName() << "(" << ports_->GetReset() << ")"
       << ", ." << sram->GetAddrPin(0) << "("
       << ArrayResource::SigName(*res, "addr") << ")"
       << ", ." << sram->GetRdataPin(0) << "("
       << ArrayResource::SigName(*res, "rdata") << ")"
       << ", ." << sram->GetWdataPin(0) << "("
       << ArrayResource::SigName(*res, "wdata") << ")"
       << ", ." << sram->GetWenPin(0) << "("
       << ArrayResource::SigName(*res, "wdata_en") << ")"
       << ");\n";
  }
  for (IResource *res : sram_if_) {
    os << "  wire " << SramIf::WenPort(res) << ";\n";
    os << "  assign " << SramIf::WenPort(res) << " = 0;\n";
  }
}

void SelfShell::WritePortConnection(ostream &os) {
  for (IResource *res : axi_) {
    axi::AxiShell shell(res);
    shell.WritePortConnection(os);
  }
  for (IResource *res : ext_input_) {
    auto *params = res->GetParams();
    string input_port;
    int width;
    params->GetExtInputPort(&input_port, &width);
    GenConnection(input_port, os);
  }
  for (IResource *res : ext_output_) {
    auto *params = res->GetParams();
    string output_port;
    int width;
    params->GetExtOutputPort(&output_port, &width);
    GenConnection(output_port, os);
  }
  for (IResource *res : ext_task_entry_) {
    string v = ExtTask::ReqValidPin(res);
    GenConnection(v, os);
  }
  for (IResource *res : ext_task_call_) {
    if (res->GetParams()->GetEmbeddedModuleFileName().empty()) {
      string req_ready = ExtTask::ReqReadyPin(res);
      string req_valid = ExtTask::ReqValidPin(res);
      GenConnection(req_ready, os);
      GenConnection(req_valid, os);
    }
  }
  for (IResource *res : ext_task_wait_) {
    string res_valid = ExtTask::ResValidPin(res->GetParentResource());
    GenConnection(res_valid, os);
    for (int i = 0; i < res->output_types_.size(); ++i) {
      string d = ExtTask::DataPin(res, i);
      GenConnection(d, os);
    }
  }
  for (IResource *res : ext_ram_) {
    GenConnection(ArrayResource::SigName(*res, "addr"), os);
    GenConnection(ArrayResource::SigName(*res, "rdata"), os);
    GenConnection(ArrayResource::SigName(*res, "wdata"), os);
    GenConnection(ArrayResource::SigName(*res, "wdata_en"), os);
  }
  for (IResource *res : sram_if_) {
    GenConnection(SramIf::WenPort(res), os);
  }
}

void SelfShell::GenConnection(const string &sig, ostream &os) {
  os << ", ." << sig << "(" << sig << ")";
}

void SelfShell::WriteShellFSM(ostream &os) {
  for (IResource *res : axi_) {
    axi::AxiShell shell(res);
    shell.WriteFSM(ports_, reset_polarity_, os);
  }
}

void SelfShell::ProcessModule(IModule *mod) {
  for (ITable *tab : mod->tables_) {
    for (IResource *res : tab->resources_) {
      ProcessResource(res);
    }
  }
}

void SelfShell::ProcessResource(IResource *res) {
  auto *klass = res->GetClass();
  if (resource::IsAxiMasterPort(*klass) || resource::IsAxiSlavePort(*klass)) {
    axi_.push_back(res);
  }
  if (resource::IsExtInput(*klass)) {
    ext_input_.push_back(res);
  }
  if (resource::IsExtOutput(*klass)) {
    ext_output_.push_back(res);
  }
  if (resource::IsExtTask(*klass)) {
    ext_task_entry_.push_back(res);
  }
  if (resource::IsExtTaskCall(*klass)) {
    ext_task_call_.push_back(res);
  }
  if (resource::IsExtTaskWait(*klass)) {
    ext_task_wait_.push_back(res);
  }
  if (resource::IsArray(*klass)) {
    IArray *ar = res->GetArray();
    if (ar != nullptr && ar->IsExternal()) {
      ext_ram_.push_back(res);
    }
  }
  if (resource::IsSramIf(*klass)) {
    sram_if_.push_back(res);
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
