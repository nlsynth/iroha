#include "writer/verilog/channel.h"

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

Channel::Channel(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void Channel::BuildResource() {
  if (resource::IsChannelWrite(*res_.GetClass())) {
    ostream &rs = tmpl_->GetStream(kRegisterSection);
    IChannel *ic = res_.GetChannel();
    rs << "  reg" << Table::WidthSpec(ic->GetValueType())
       << " " << DataPort(*ic) << ";\n";
    rs << "  reg " << EnPort(*ic) << ";\n";
    ostream &is = tab_.InitialValueSectionStream();
    is << "      " << DataPort(*ic) << " <= 0;\n"
       << "      " << EnPort(*ic) << " <= 0;\n";
  }
  if (resource::IsChannelRead(*res_.GetClass())) {
    ostream &rs = tmpl_->GetStream(kRegisterSection);
    IChannel *ic = res_.GetChannel();
    rs << "  reg " << AckPort(*ic) << ";\n";
    ostream &is = tab_.InitialValueSectionStream();
    is << "      " << AckPort(*ic) << " <= 0;\n";
  }
}

void Channel::BuildInsn(IInsn *insn, State *st) {
  static const char I[] = "          ";
  ostream &os = st->StateBodySectionStream();
  string insn_st = InsnWriter::MultiCycleStateName(*(insn->GetResource()));
  if (resource::IsChannelWrite(*res_.GetClass())) {
    os << I << "// Channel write\n"
       << I << "if (" << insn_st << " == 0) begin\n";
    IChannel *ic = res_.GetChannel();
    CHECK(ic);
    os << I << "  " << DataPort(*ic) << " <= " << InsnWriter::RegisterValue(*insn->inputs_[0], tab_.GetNames()) << ";\n";
    os << I << "  if (" << AckPort(*ic) << ") begin\n"
       << I << "    " << insn_st << " <= 3;\n"
       << I << "    " << EnPort(*ic) << " <= 0;\n"
       << I << "  end else begin\n"
       << I << "    " << EnPort(*ic) << " <= 1;\n"
       << I << "  end\n";
    os << I << "end\n";
  }
  if (resource::IsChannelRead(*res_.GetClass())) {
    IChannel *ic = res_.GetChannel();
    CHECK(ic);
    os << I << "// Channel read\n"
       << I << "if (" << insn_st << " == 0) begin\n"
       << I << "  if (" << EnPort(*ic) << ") begin\n"
       << I << "    " << insn_st << " <= 3;\n"
       << I << "    " << AckPort(*ic) << " <= 1;\n"
       << I << "  end\n"
       << I << "end\n";
    os << I << "if (" << insn_st << " == 3) begin\n"
       << I << "    " << AckPort(*ic) << " <= 0;\n"
       << I << "end\n";
    ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
    ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = " << DataPort(*ic) << ";\n";
  }
}

string Channel::DataPort(const IChannel &ic) {
  return PortName(ic, ic.GetParams()->GetChannelDataPort(), "data");
}

string Channel::AckPort(const IChannel &ic) {
  return PortName(ic, ic.GetParams()->GetChannelAckPort(), "ack");
}

string Channel::EnPort(const IChannel &ic) {
  return PortName(ic, ic.GetParams()->GetChannelEnPort(), "en");
}

string Channel::PortName(const IChannel &ic, const string &dflt,
			 const string &type) {
  if (!dflt.empty()) {
    return dflt;
  }
  string suffix = type + "_" + Util::Itoa(ic.GetId());
  if (ic.GetReader() != nullptr) {
      if (ic.GetWriter() != nullptr) {
	return "channel_" + suffix;
      } else {
	return "ext_r_" + suffix;
      }
  } else {
    return "ext_w_" + suffix;
  }
}

void Channel::BuildChannelPorts(const ChannelInfo &ci, Ports *ports) {
  for (auto *ch : ci.upward_) {
    int width = ch->GetValueType().GetWidth();
    ports->AddPort(Channel::DataPort(*ch), Port::OUTPUT_WIRE, width);
    ports->AddPort(Channel::EnPort(*ch), Port::OUTPUT_WIRE, 0);
    ports->AddPort(Channel::AckPort(*ch), Port::INPUT, 0);
  }
  for (auto *ch : ci.downward_) {
    int width = ch->GetValueType().GetWidth();
    ports->AddPort(Channel::DataPort(*ch), Port::INPUT, width);
    ports->AddPort(Channel::EnPort(*ch), Port::INPUT, 0);
    ports->AddPort(Channel::AckPort(*ch), Port::OUTPUT_WIRE, 0);
  }
}

void Channel::BuildRootWire(const ChannelInfo &ci, Module *module) {
  ModuleTemplate *tmpl = module->GetModuleTemplate();
  for (auto *ch : ci.common_root_) {
    // This doesn't assume in-module channels.
    IResource *writer_res = ch->GetWriter();
    const IModule *writer_mod = nullptr;
    if (writer_res != nullptr) {
      writer_mod = writer_res->GetTable()->GetModule();
    }
    IResource *reader_res = ch->GetReader();
    const IModule *reader_mod = nullptr;
    if (reader_res != nullptr) {
      reader_mod = reader_res->GetTable()->GetModule();
    }
    // (1) Writer is in this module. Reader is in sub module.
    if (writer_mod == module->GetIModule()) {
      ostream &ws = tmpl->GetStream(kInsnWireDeclSection);
      ws << "  wire " << AckPort(*ch) << ";\n";
    }
    // (2) Reader is in this module. Writer is in sub module.
    if (reader_mod == module->GetIModule()) {
      ostream &ws = tmpl->GetStream(kInsnWireDeclSection);
      ws << "  wire " << EnPort(*ch) << ";\n"
	 << "  reg" << Table::WidthSpec(ch->GetValueType())
	 << " " << DataPort(*ch) << ";\n";
    }
    // (3) Both reader and writer is in sub module.
    if (writer_mod != nullptr && reader_mod != nullptr) {
      if (writer_mod != module->GetIModule() &&
	  reader_mod != module->GetIModule()) {
	ostream &ws = tmpl->GetStream(kInsnWireDeclSection);
	ws << "  wire " << AckPort(*ch) << ";\n";
	ws << "  wire " << EnPort(*ch) << ";\n"
	   << "  wire " << Table::WidthSpec(ch->GetValueType())
	   << " " << DataPort(*ch) << ";\n";
      }
    }
  }
}

void Channel::BuildChildChannelWire(const ChannelInfo &ci,
				    const IModule *child_mod,
				    ostream &os) {
  auto it = ci.child_upward_.find(child_mod);
  if (it != ci.child_upward_.end()) {
    for (auto *ch : it->second) {
      BuildChildModuleChannelWire(*ch, os);
    }
  }
  it = ci.child_downward_.find(child_mod);
  if (it != ci.child_downward_.end()) {
    for (auto *ch : it->second) {
      BuildChildModuleChannelWire(*ch, os);
    }
  }
}

void Channel::BuildChildModuleChannelWire(const IChannel &ch, ostream &is) {
  string port = DataPort(ch);
  is << ", ." << port << "(" << port << ")";
  port = AckPort(ch);
  is << ", ." << port << "(" << port << ")";
  port = EnPort(ch);
  is << ", ." << port << "(" << port << ")";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
