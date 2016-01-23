#include "writer/verilog/module.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "iroha/stl_util.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

Module::Module(const IModule *i_mod, const Connection &conn, Embed *embed)
  : i_mod_(i_mod), conn_(conn), embed_(embed) {
  tmpl_.reset(new ModuleTemplate);
  ports_.reset(new Ports);
  reset_polarity_ = i_mod_->GetDesign()->GetParams()->GetResetPolarity();
}

Module::~Module() {
  STLDeleteValues(&tables_);
  STLDeleteValues(&srams_);
}

void Module::Write(ostream &os) {
  os << "module " << i_mod_->GetName() << "(";
  ports_->Output(Ports::PORT_NAME, os);
  os << ");\n";
  ports_->Output(Ports::PORT_TYPE, os);
  os << "\n";

  os << tmpl_->GetContents(kStateDeclSection);
  os << tmpl_->GetContents(kStateVarSection);
  os << tmpl_->GetContents(kRegisterSection);
  os << tmpl_->GetContents(kResourceSection);
  os << tmpl_->GetContents(kInsnWireDeclSection);
  os << tmpl_->GetContents(kInsnWireValueSection);
  os << "\n";

  for (auto *tab : tables_) {
    tab->Write(os);
  }
  os << tmpl_->GetContents(kEmbeddedInstanceSection);
  os << tmpl_->GetContents(kSubModuleSection);
  os << "\nendmodule\n";
}

bool Module::GetResetPolarity() const {
  return reset_polarity_;
}

const IModule *Module::GetIModule() const {
  return i_mod_;
}

const Ports *Module::GetPorts() const {
  return ports_.get();
}

void Module::Build() {
  ports_->AddPort("clk", Port::INPUT_CLK, 0);
  if (reset_polarity_) {
    ports_->AddPort("rst", Port::INPUT_RESET, 0);
  } else {
    ports_->AddPort("rst_n", Port::INPUT_RESET, 0);
  }

  int nth = 0;
  for (auto *i_table : i_mod_->tables_) {
    Table *tab = new Table(i_table, ports_.get(), this, embed_,
			   tmpl_.get(), nth);
    tab->Build();
    tables_.push_back(tab);
    ++nth;
  }

  const ChannelInfo *ci = conn_.GetConnectionInfo(i_mod_);
  if (ci != nullptr) {
    BuildChannelConnections(*ci);
  }
}

void Module::BuildChildModuleSection(vector<Module *> &mods) {
  ostream &is = tmpl_->GetStream(kSubModuleSection);
  for (auto *mod : mods) {
    const IModule *imod = mod->GetIModule();
    is << "  " << imod->GetName() << " "
       << "inst_" << imod->GetName() << "(";
    is << "." << ports_->GetClk() << "(" << mod->GetPorts()->GetClk() << ")";
    is << ", ." << ports_->GetReset() << "("
       << mod->GetPorts()->GetClk() << ")";
    const ChannelInfo *ci = conn_.GetConnectionInfo(imod);
    if (ci != nullptr) {
      for (auto *ch : ci->ext_writer_) {
	BuildChildModuleChannelWire(*ch, is);
      }
      for (auto *ch : ci->ext_writer_path_) {
	BuildChildModuleChannelWire(*ch, is);
      }
      for (auto *ch : ci->ext_reader_) {
	BuildChildModuleChannelWire(*ch, is);
      }
      for (auto *ch : ci->ext_reader_path_) {
	BuildChildModuleChannelWire(*ch, is);
      }
      for (auto *ch : ci->data_path_) {
	BuildChildModuleChannelWire(*ch, is);
      }
      for (auto *ch : ci->reader_from_up_) {
	BuildChildModuleChannelWire(*ch, is);
      }
    }
    is << ");\n";
  }
}

InternalSRAM *Module::RequestInternalSRAM(const IResource &res) {
  InternalSRAM *sram = new InternalSRAM(*this, res);
  srams_.push_back(sram);
  return sram;
}

const vector<InternalSRAM *> &Module::GetInternalSRAMs() const {
  return srams_;
}

void Module::BuildChildModuleChannelWire(const IChannel &ch, ostream &is) {
  string port = InsnWriter::ChannelDataPort(ch);
  is << ", ." << port << "(" << port << ")";
}

void Module::BuildChannelConnections(const ChannelInfo &ci) {
  for (auto *ch : ci.ext_writer_) {
    int width = ch->GetValueType().GetWidth();
    ports_->AddPort(InsnWriter::ChannelDataPort(*ch), Port::OUTPUT, width);
  }
  for (auto *ch : ci.ext_writer_path_) {
    int width = ch->GetValueType().GetWidth();
    ports_->AddPort(InsnWriter::ChannelDataPort(*ch), Port::OUTPUT_WIRE, width);
  }
  for (auto *ch : ci.ext_reader_) {
    int width = ch->GetValueType().GetWidth();
    ports_->AddPort(InsnWriter::ChannelDataPort(*ch), Port::INPUT, width);
  }
  for (auto *ch : ci.ext_reader_path_) {
    int width = ch->GetValueType().GetWidth();
    ports_->AddPort(InsnWriter::ChannelDataPort(*ch), Port::INPUT, width);
  }
  for (auto *ch : ci.writer_to_down_) {
    int width = ch->GetValueType().GetWidth();
    ostream &rs = tmpl_->GetStream(kRegisterSection);
    rs << "  reg ";
    if (width > 0) {
      rs << "[" << (width - 1) << ":0] ";
    }
    rs << InsnWriter::ChannelDataPort(*ch) << ";\n";
  }
  for (auto *ch : ci.reader_from_up_) {
    int width = ch->GetValueType().GetWidth();
    ports_->AddPort(InsnWriter::ChannelDataPort(*ch), Port::INPUT, width);
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
