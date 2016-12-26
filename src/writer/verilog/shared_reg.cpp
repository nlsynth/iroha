#include "writer/verilog/shared_reg.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

SharedReg::SharedReg(const IResource &res, const Table &table)
  : Resource(res, table), has_default_output_value_(false),
    default_output_value_(0), writers_(nullptr),
    need_write_arbitration_(false) {
  auto *klass = res_.GetClass();
  if (resource::IsSharedReg(*klass)) {
    auto *params = res_.GetParams();
    string unused;
    params->GetExtOutputPort(&unused, &width_);
    has_default_output_value_ =
      params->GetDefaultValue(&default_output_value_);
    writers_ = table.GetModule()->GetConnection().GetSharedRegWriters(&res_);
    if (writers_ != nullptr || has_default_output_value_) {
      need_write_arbitration_ = true;
    }
  } else if (resource::IsSharedRegWriter(*klass)) {
    auto *params = res_.GetSharedRegister()->GetParams();
    string unused;
    params->GetExtOutputPort(&unused, &width_);
  } else {
    width_ = 0;
  }
}

void SharedReg::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsSharedReg(*klass)) {
    BuildSharedRegResource();
  }
  if (resource::IsSharedRegWriter(*klass)) {
    BuildSharedRegWriterResource();
  }
}

void SharedReg::BuildSharedRegResource() {
  ostream &rs = tmpl_->GetStream(kRegisterSection);
  rs << "  // shared-reg\n";
  rs << "  reg ";
  if (width_ > 0) {
    rs << "[" << width_ - 1 << ":0]";
  }
  rs << " " << RegName(res_) << ";\n";
  if (need_write_arbitration_) {
    // Priorities are
    // (1) Writers in this table
    // (2) Writers in other table
    // (3) Default output value
    ostream &os = tab_.StateOutputSectionStream();
    os << "      " << RegName(res_) << " <= ";
    string value;
    if (has_default_output_value_) {
      value = Util::Itoa(default_output_value_);
    } else {
      value = RegName(res_);
    }
    if (writers_ != nullptr) {
      for (auto *res : *writers_) {
	value = WriterEnName(*res) + " ? " + WriterName(*res) + " : (" + value + ")";
      }
    }
    os << SelectValueByState(value);
    os << ";\n";
  }
  // Reset value
  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << RegName(res_) << " <= ";
  if (has_default_output_value_) {
    is << default_output_value_;
  } else {
    is << 0;
  }
  is << ";\n";
  // Read wires from shared-reg
  // (on the other hand, write wires are wired from shared-reg-writer)
  BuildReadWire();
}

void SharedReg::BuildSharedRegWriterResource() {
  ostream &rs = tmpl_->GetStream(kRegisterSection);
  rs << "  // shared-reg-writer\n";
  rs << "  reg ";
  if (width_ > 0) {
    rs << "[" << width_ - 1 << ":0]";
  }
  rs << " " << WriterName(res_) << ";\n";
  rs << "  reg " << WriterEnName(res_) << ";\n";
  // Reset value
  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << WriterName(res_) << " <= 0;\n"
     << "      " << WriterEnName(res_) << " <= 0;\n";
  // Write en signal.
  ostream &os = tab_.StateOutputSectionStream();
  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);
  os << "      " << WriterEnName(res_) << " <= ";
  WriteStateUnion(callers, os);
  os << ";\n";

  BuildWriteWire(&res_);
}

void SharedReg::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsSharedReg(*klass)) {
    if (!need_write_arbitration_ &&
	insn->inputs_.size() == 1) {
      ostream &os = st->StateBodySectionStream();
      os << "          " << RegName(res_) << " <= "
	 << InsnWriter::RegisterValue(*insn->inputs_[0], tab_.GetNames())
	 << ";\n";
    }
    if (insn->outputs_.size() == 1) {
      // Read from self.
      ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
      ws << "  assign "
	 << InsnWriter::InsnOutputWireName(*insn, 0)
	 << " = "
	 << RegName(res_) << ";\n";
    }
  }
  if (resource::IsSharedRegReader(*klass)) {
    // Read from another table.
    IResource *source = res_.GetSharedRegister();
    ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
    ws << "  assign "
       << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = "
       << RegName(*source) << ";\n";
  }
  if (resource::IsSharedRegWriter(*klass)) {
    ostream &os = st->StateBodySectionStream();
    os << "          " << WriterName(res_) << " <= "
       << InsnWriter::RegisterValue(*insn->inputs_[0], tab_.GetNames())
       << ";\n";
  }
}

string SharedReg::WriterName(const IResource &res) {
  return RegName(res) + "_w";
}

string SharedReg::WriterEnName(const IResource &res) {
  return WriterName(res) + "_en";
}

string SharedReg::RegName(const IResource &res) {
  auto *params = res.GetParams();
  int unused_width;
  string port_name;
  params->GetExtOutputPort(&port_name, &unused_width);
  ITable *tab = res.GetTable();
  IModule *mod = tab->GetModule();
  return "shared_reg_" +
    Util::Itoa(mod->GetId()) + "_" +
    Util::Itoa(tab->GetId()) + "_" + Util::Itoa(res.GetId()) +
    "_" + port_name;
}

void SharedReg::AddChildWire(const IResource *res, bool is_write, ostream &os) {
  string name;
  if (is_write) {
    name = WriterName(*res);
  } else {
    name = RegName(*res);
  }
  os << ", ." << name << "(" << name << ")";
  if (is_write) {
    name = WriterEnName(*res);
    os << ", ." << name << "(" << name << ")";
  }
}

void SharedReg::BuildReadWire() {
  auto *readers = tab_.GetModule()->GetConnection().GetSharedRegReaders(&res_);
  if (readers == nullptr) {
    return;
  }
  IModule *reg_module = res_.GetTable()->GetModule();
  set<const IModule *> wired_modules;
  set<const IModule *> has_upward;
  set<const IModule *> has_downward;
  for (auto *reader : *readers) {
    IModule *reader_module = reader->GetTable()->GetModule();
    const IModule *common_root = Connection::GetCommonRoot(reg_module,
							   reader_module);
    if (reader_module != common_root && reg_module != common_root) {
      if (wired_modules.find(common_root) == wired_modules.end()) {
	AddWire(common_root, &res_, false);
	wired_modules.insert(common_root);
      }
    }
    // upward
    for (IModule *imod = reg_module; imod != common_root;
	 imod = imod->GetParentModule()) {
      if (has_upward.find(imod) == has_upward.end()) {
	AddReadPort(imod, &res_, true);
	has_upward.insert(imod);
      }
    }
    // downward
    for (IModule *imod = reader_module; imod != common_root;
	 imod = imod->GetParentModule()) {
      if (has_downward.find(imod) == has_downward.end()) {
	AddReadPort(imod, &res_, false);
	has_downward.insert(imod);
      }
    }
  }
}

void SharedReg::BuildWriteWire(const IResource *writer) {
  IResource *reg = writer->GetSharedRegister();
  IModule *reg_module = reg->GetTable()->GetModule();
  IModule *writer_module = writer->GetTable()->GetModule();
  const IModule *common_root = Connection::GetCommonRoot(reg_module,
							 writer_module);
  if (writer_module != common_root) {
    AddWire(common_root, writer, true);
  }
  // downward
  for (IModule *imod = reg_module; imod != common_root;
       imod = imod->GetParentModule()) {
    AddWritePort(imod, writer, false);
  }
  // upward
  for (IModule *imod = writer_module; imod != common_root;
       imod = imod->GetParentModule()) {
    AddWritePort(imod, writer, true);
  }
}

void SharedReg::AddWire(const IModule *imod, const IResource *accessor,
			bool is_write) {
  Module *mod = tab_.GetModule()->GetByIModule(imod);
  auto *tmpl = mod->GetModuleTemplate();
  ostream &rs = tmpl->GetStream(kResourceSection);
  int width;
  if (is_write) {
    width = accessor->GetSharedRegister()->GetParams()->GetWidth();
  } else {
    width = accessor->GetParams()->GetWidth();
  }
  rs << "  wire ";
  if (width > 0) {
    rs << "[" << width - 1 << ":0] ";
  }
  if (is_write) {
    rs << WriterName(*accessor) << ";\n";
  } else {
    rs << RegName(*accessor) << ";\n";
  }
  if (is_write) {
    rs << "  wire " << WriterEnName(*accessor) << ";\n";
  }
}

void SharedReg::AddReadPort(const IModule *imod, const IResource *reader,
			    bool upward) {
  Module *mod = tab_.GetModule()->GetByIModule(imod);
  Ports *ports = mod->GetPorts();
  int width = reader->GetParams()->GetWidth();
  if (upward) {
    ports->AddPort(RegName(*reader), Port::OUTPUT_WIRE, width);
  } else {
    ports->AddPort(RegName(*reader), Port::INPUT, width);
  }
  Module *parent_mod = mod->GetParentModule();
  ostream &os = parent_mod->ChildModuleInstSectionStream(mod);
  AddChildWire(reader, false, os);
}

void SharedReg::AddWritePort(const IModule *imod, const IResource *writer,
			     bool upward) {
  Module *mod = tab_.GetModule()->GetByIModule(imod);
  Ports *ports = mod->GetPorts();
  int width = writer->GetSharedRegister()->GetParams()->GetWidth();
  if (upward) {
    ports->AddPort(WriterName(*writer), Port::OUTPUT_WIRE, width);
    ports->AddPort(WriterEnName(*writer), Port::OUTPUT_WIRE, 0);
  } else {
    ports->AddPort(WriterName(*writer), Port::INPUT, width);
    ports->AddPort(WriterEnName(*writer), Port::INPUT, 0);
  }
  Module *parent_mod = mod->GetParentModule();
  ostream &os = parent_mod->ChildModuleInstSectionStream(mod);
  AddChildWire(writer, true, os);
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
