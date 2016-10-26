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
    auto *params = res_.GetSharedReg()->GetParams();
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
}

void SharedReg::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsSharedReg(*klass)) {
    if (!need_write_arbitration_ &&
	insn->inputs_.size() == 1) {
      ostream &os = st->StateBodySectionStream();
      os << "          " << RegName(res_) << " <= "
	 << InsnWriter::RegisterName(*insn->inputs_[0])
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
    IResource *source = res_.GetSharedReg();
    ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
    ws << "  assign "
       << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = "
       << RegName(*source) << ";\n";
  }
  if (resource::IsSharedRegWriter(*klass)) {
    ostream &os = st->StateBodySectionStream();
    os << "          " << WriterName(res_) << " <= "
       << InsnWriter::RegisterName(*insn->inputs_[0]) << ";\n";
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

void SharedReg::BuildReaderPorts(const SharedRegConnectionInfo &pi,
				 Ports *ports) {
  for (IResource *res : pi.has_upward_port) {
    int width = res->GetParams()->GetWidth();
    ports->AddPort(RegName(*res), Port::OUTPUT_WIRE, width);
  }
  for (IResource *res : pi.has_downward_port) {
    int width = res->GetParams()->GetWidth();
    ports->AddPort(RegName(*res), Port::INPUT, width);
  }
}

void SharedReg::BuildWriterPorts(const SharedRegConnectionInfo &pi,
				 Ports *ports) {
  for (IResource *res : pi.has_upward_port) {
    int width = res->GetSharedReg()->GetParams()->GetWidth();
    ports->AddPort(WriterName(*res), Port::OUTPUT_WIRE, width);
    ports->AddPort(WriterEnName(*res), Port::OUTPUT_WIRE, 0);
  }
  for (IResource *res : pi.has_downward_port) {
    int width = res->GetSharedReg()->GetParams()->GetWidth();
    ports->AddPort(WriterName(*res), Port::INPUT, width);
    ports->AddPort(WriterEnName(*res), Port::INPUT, 0);
  }
}

void SharedReg::BuildReaderChildWire(const SharedRegConnectionInfo &pi,
				     ostream &os) {
  for (IResource *res : pi.has_upward_port) {
    AddChildWire(res, false, os);
  }
  for (IResource *res : pi.has_downward_port) {
    AddChildWire(res, false, os);
  }
}

void SharedReg::BuildWriterChildWire(const SharedRegConnectionInfo &pi,
				     ostream &os) {
  for (IResource *res : pi.has_upward_port) {
    AddChildWire(res, true, os);
  }
  for (IResource *res : pi.has_downward_port) {
    AddChildWire(res, true, os);
  }
}

void SharedReg::AddChildWire(IResource *res, bool is_write, ostream &os) {
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

void SharedReg::BuildReaderRootWire(const SharedRegConnectionInfo &pi,
				    Module *module) {
  BuildRootWire(pi, false, module);
}

void SharedReg::BuildWriterRootWire(const SharedRegConnectionInfo &pi,
				    Module *module) {
  BuildRootWire(pi, true, module);
}

void SharedReg::BuildRootWire(const SharedRegConnectionInfo &pi,
			      bool is_write,
			      Module *module) {
  ModuleTemplate *tmpl = module->GetModuleTemplate();
  ostream &ws = tmpl->GetStream(kInsnWireDeclSection);
  for (IResource *res : pi.has_wire) {
    ws << "  wire ";
    int width;
    if (is_write) {
      width = res->GetSharedReg()->GetParams()->GetWidth();
    } else {
      width = res->GetParams()->GetWidth();
    }
    if (width > 0) {
      ws << "[" << width - 1 << ":0] ";
    }
    if (is_write) {
      ws << WriterName(*res) << ";\n";
    } else {
      ws << RegName(*res) << ";\n";
    }
    if (is_write) {
      ws << "  wire " << WriterEnName(*res) << ";\n";
    }
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
