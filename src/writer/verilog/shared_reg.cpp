// A shared register can have shared register writers, readers and
// dataflow entries (equivalent to readers).
//
// shared-reg-writer generates connections from shared-reg-writer to shared-reg.
// shared-reg generates connections from shared-reg to shared-reg-reader.

#include "writer/verilog/shared_reg.h"

#include "design/design_util.h"
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
#include "writer/verilog/shared_reg_accessor.h"

namespace iroha {
namespace writer {
namespace verilog {

SharedReg::SharedReg(const IResource &res, const Table &table)
  : Resource(res, table), has_default_output_value_(false),
    default_output_value_(0),
    need_write_arbitration_(false) {
  auto *klass = res_.GetClass();
  CHECK(resource::IsSharedReg(*klass));
  auto *params = res_.GetParams();
  string unused;
  params->GetExtOutputPort(&unused, &width_);
  has_default_output_value_ =
    params->GetDefaultValue(&default_output_value_);
  auto &readers =
    table.GetModule()->GetConnection().GetSharedRegReaders(&res_);
  for (auto *r : readers) {
    readers_.push_back(r);
  }
  auto &children =
    table.GetModule()->GetConnection().GetSharedRegChildren(&res_);
  for (auto *r : children) {
    readers_.push_back(r);
  }
  writers_ = table.GetModule()->GetConnection().GetSharedRegWriters(&res_);
  if (writers_.size() > 0 || has_default_output_value_) {
    need_write_arbitration_ = true;
  }
  GetOptions(&use_notify_, &use_mailbox_);
}

void SharedReg::BuildResource() {
  ostream &rs = tmpl_->GetStream(kRegisterSection);
  ostream &is = tab_.InitialValueSectionStream();
  rs << "  // shared-reg";
  if (use_notify_) {
    rs << " use-notify";
  }
  if (use_mailbox_) {
    rs << " use-mailbox";
  }
  rs << "\n";
  rs << "  reg ";
  if (width_ > 0) {
    rs << "[" << width_ - 1 << ":0]";
  }
  rs << " " << RegName(res_) << ";\n";
  // Reset value
  is << "      " << RegName(res_) << " <= ";
  if (has_default_output_value_) {
    is << default_output_value_;
  } else {
    is << 0;
  }
  is << ";\n";
  if (use_notify_) {
    rs << "  reg " << RegNotifierName(res_) << ";\n";
    vector<string> notifiers;
    for (auto *writer : writers_) {
      if (SharedRegAccessor::UseNotify(writer)) {
	notifiers.push_back(WriterNotifierName(*writer));
      }
    }
    ostream &os = tab_.StateOutputSectionStream();
    os << "      " << RegNotifierName(res_)
       << " <= ";
    if (notifiers.size() > 0) {
      os << Util::Join(notifiers, " | ");
    } else {
      os << "0";
    }
    os << ";\n";
    is << "      " << RegNotifierName(res_) << " <= 0;\n";
  }
  if (use_mailbox_) {
    BuildMailbox();
  }
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
    for (auto *writer : writers_) {
      string en = WriterEnName(*writer);
      bool n, m;
      SharedRegAccessor::GetAccessorFeatures(writer, &n, &m);
      if (m) {
	// Writes the value only when put to the mailbox is granted.
	// en && (!req || ack)
	en = "(" + en + " && (!" + RegMailboxPutReqName(*writer) +
	  " || " + RegMailboxPutAckName(*writer) + "))";
      }
      value = en + " ? " + WriterName(*writer) + " : (" + value + ")";
    }
    os << SelectValueByState(value);
    os << ";\n";
  }
  // Read wires from shared-reg
  // (on the other hand, write wires are wired from shared-reg-writer)
  BuildReadWire();
}

void SharedReg::BuildMailbox() {
  ostream &rs = tmpl_->GetStream(kRegisterSection);
  rs << "  reg " << RegMailboxName(res_) << ";\n";
  vector<string> put_reqs;
  if (writers_.size() > 0) {
    for (auto *writer : writers_) {
      if (!SharedRegAccessor::UseMailbox(writer)) {
	continue;
      }
      rs << "  wire " << RegMailboxPutAckName(*writer) << ";\n";
      rs << "  assign " << RegMailboxPutAckName(*writer) << " = "
	 << "(!" << RegMailboxName(res_) << ") && ";
      if (put_reqs.size() > 0) {
	rs << "(!(" << Util::Join(put_reqs, " | ") << ")) && ";
      }
      rs << RegMailboxPutReqName(*writer) << ";\n";
      put_reqs.push_back(RegMailboxPutReqName(*writer));
    }
  } else {
    put_reqs.push_back("0");
  }
  vector<string> get_reqs;
  if (readers_.size() > 0) {
    for (auto *reader : readers_) {
      if (!SharedRegAccessor::UseMailbox(reader)) {
	continue;
      }
      rs << "  wire " << RegMailboxGetAckName(*reader) << ";\n";
      rs << "  assign " << RegMailboxGetAckName(*reader) << " = "
	 << "(" << RegMailboxName(res_) << ") && ";
      if (get_reqs.size() > 0) {
	rs << "(!(" << Util::Join(get_reqs, " | ") << ")) && ";
      }
      rs << RegMailboxGetReqName(*reader) << ";\n";
      get_reqs.push_back(RegMailboxGetReqName(*reader));
    }
  } else {
    get_reqs.push_back("0");
  }
  ostream &os = tab_.StateOutputSectionStream();
  os << "      if (" << RegMailboxName(res_) << ") begin\n"
     << "        " << RegMailboxName(res_) << " <= "
     << "!(" << Util::Join(get_reqs, " | ") << ");\n"
     << "      end else begin\n"
     << "        " << RegMailboxName(res_) << " <= "
     << Util::Join(put_reqs, " | ") << ";\n"
     << "      end\n";
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
}

string SharedReg::WriterName(const IResource &writer) {
  return RegName(writer) + "_w";
}

string SharedReg::WriterEnName(const IResource &writer) {
  return WriterName(writer) + "_en";
}

string SharedReg::RegNotifierName(const IResource &reg) {
  return RegName(reg) + "_notify";
}

string SharedReg::WriterNotifierName(const IResource &writer) {
  return RegName(writer) + "_write_notify";
}

string SharedReg::RegMailboxName(const IResource &reg) {
  return RegName(reg) + "_mailbox";
}

string SharedReg::RegMailboxPutReqName(const IResource &writer) {
  return RegName(writer) + "_mailbox_put_req";
}

string SharedReg::RegMailboxPutAckName(const IResource &writer) {
  return RegName(writer) + "_mailbox_put_ack";
}

string SharedReg::RegMailboxGetReqName(const IResource &reader) {
  return RegName(reader) + "_mailbox_get_req";
}

string SharedReg::RegMailboxGetAckName(const IResource &reader) {
  return RegName(reader) + "_mailbox_get_ack";
}

string SharedReg::RegMailboxGetBufName(const IResource &reader) {
  return RegName(reader) + "_mailbox_get_buf";
}

string SharedReg::RegName(const IResource &reg) {
  auto *params = reg.GetParams();
  int unused_width;
  string port_name;
  if (params != nullptr) {
    params->GetExtOutputPort(&port_name, &unused_width);
  }
  ITable *tab = reg.GetTable();
  IModule *mod = tab->GetModule();
  string n = "shared_reg_" +
    Util::Itoa(mod->GetId()) + "_" +
    Util::Itoa(tab->GetId()) + "_" + Util::Itoa(reg.GetId());
  if (!port_name.empty()) {
    n += "_" + port_name;
  }
  return n;
}

void SharedReg::AddChildWire(const IResource *accessor, bool is_write,
			     bool use_notify, bool use_mailbox,
			     ostream &os) {
  const IResource *reg = accessor->GetParentResource();
  vector<string> names;
  string name;
  if (is_write) {
    names.push_back(WriterName(*accessor));
  } else {
    names.push_back(RegName(*reg));
  }
  if (is_write) {
    names.push_back(WriterEnName(*accessor));
  }
  if (use_notify) {
    if (is_write) {
      names.push_back(WriterNotifierName(*accessor));
    } else {
      names.push_back(RegNotifierName(*reg));
    }
  }
  if (use_mailbox) {
    if (is_write) {
      names.push_back(RegMailboxPutReqName(*accessor));
    } else {
      names.push_back(RegMailboxGetReqName(*accessor));
    }
    if (is_write) {
      names.push_back(RegMailboxPutAckName(*accessor));
    } else {
      names.push_back(RegMailboxGetAckName(*accessor));
    }
  }
  for (const string &s : names) {
    os << ", ." << s << "(" << s << ")";
  }
}

void SharedReg::BuildReadWire() {
  IModule *reg_module = res_.GetTable()->GetModule();
  set<const IModule *> wired_modules;
  set<const IModule *> has_upward;
  set<const IModule *> has_downward;
  for (auto *reader : readers_) {
    IModule *reader_module = reader->GetTable()->GetModule();
    const IModule *common_root = Connection::GetCommonRoot(reg_module,
							   reader_module);
    if (reader_module != common_root && reg_module != common_root) {
      if (wired_modules.find(common_root) == wired_modules.end()) {
	AddAccessorSignals(common_root, &tab_, reader, true);
	wired_modules.insert(common_root);
      }
    }
    // upward
    for (IModule *imod = reg_module; imod != common_root;
	 imod = imod->GetParentModule()) {
      if (has_upward.find(imod) == has_upward.end()) {
	AddReadPort(imod, reader, true);
	has_upward.insert(imod);
      }
    }
    // downward
    for (IModule *imod = reader_module; imod != common_root;
	 imod = imod->GetParentModule()) {
      if (has_downward.find(imod) == has_downward.end()) {
	AddReadPort(imod, reader, false);
	has_downward.insert(imod);
      }
    }
  }
}

void SharedReg::AddAccessorSignals(const IModule *imod, const Table *tab,
				   const IResource *accessor, bool wire_only) {
  // NOTE: SharedReg::BuildResource() may have some dups of this code.
  bool is_writer = resource::IsSharedRegWriter(*(accessor->GetClass()));
  const IResource *reg = accessor->GetParentResource();
  bool same_module = false;
  if (reg->GetTable()->GetModule() == accessor->GetTable()->GetModule()) {
    same_module = true;
  }
  Module *mod = tab->GetModule()->GetByIModule(imod);
  auto *tmpl = mod->GetModuleTemplate();
  ostream &rs = tmpl->GetStream(kResourceSection);
  int width = accessor->GetParentResource()->GetParams()->GetWidth();
  string drive_by_writer = "wire";
  string drive_by_reader = "wire";
  if (!wire_only) {
    if (is_writer) {
      drive_by_writer = "reg";
    } else {
      drive_by_reader = "reg";
    }
  }
  if (is_writer || !same_module) {
    rs << "  wire "
       << Table::WidthSpec(width);
    if (is_writer) {
      rs << WriterName(*accessor) << ";\n";
    } else {
      rs << RegName(*reg) << ";\n";
    }
  }
  if (is_writer) {
    rs << "  wire " << WriterEnName(*accessor) << ";\n";
  }
  bool notify = SharedRegAccessor::UseNotify(accessor);
  if (notify) {
    if (is_writer) {
      rs << "  " << drive_by_writer << " "
	 << WriterNotifierName(*accessor) << ";\n";
    } else {
      if (!same_module) {
	rs << "  " << drive_by_writer << " "
	   << RegNotifierName(*reg) << ";\n";
      }
    }
  }
  bool mb = SharedRegAccessor::UseMailbox(accessor);
  if (mb) {
    if (is_writer) {
      if (!same_module) {
	rs << "  wire "
	   << RegMailboxPutReqName(*accessor) << ";\n"
	   << "  wire "
	   << RegMailboxPutAckName(*accessor) << ";\n";
      }
    } else {
      if (!same_module) {
	rs << "  wire "
	   << RegMailboxGetReqName(*accessor) << ";\n"
	   << "  wire "
	   << RegMailboxGetAckName(*accessor) << ";\n";
      }
    }
  }
}

void SharedReg::AddReadPort(const IModule *imod, const IResource *reader,
			    bool upward) {
  Module *mod = tab_.GetModule()->GetByIModule(imod);
  Ports *ports = mod->GetPorts();
  const IResource *reg = reader->GetParentResource();
  int width = reg->GetParams()->GetWidth();
  if (upward) {
    ports->AddPort(RegName(*reg), Port::OUTPUT_WIRE, width);
  } else {
    ports->AddPort(RegName(*reg), Port::INPUT, width);
  }
  Module *parent_mod = mod->GetParentModule();
  ostream &os = parent_mod->ChildModuleInstSectionStream(mod);
  bool use_notify = SharedRegAccessor::UseNotify(reader);
  if (use_notify) {
    if (upward) {
      ports->AddPort(RegNotifierName(*reg), Port::OUTPUT_WIRE, 0);
    } else {
      ports->AddPort(RegNotifierName(*reg), Port::INPUT, 0);
    }
  }
  bool use_mailbox = SharedRegAccessor::UseMailbox(reader);
  if (use_mailbox) {
    if (upward) {
      ports->AddPort(RegMailboxPutReqName(*reader), Port::INPUT, 0);
      ports->AddPort(RegMailboxPutAckName(*reader), Port::OUTPUT_WIRE, 0);
    } else {
      ports->AddPort(RegMailboxPutReqName(*reader), Port::OUTPUT_WIRE, 0);
      ports->AddPort(RegMailboxPutAckName(*reader), Port::INPUT, 0);
    }
  }
  AddChildWire(reader, false, use_notify, use_mailbox, os);
}

void SharedReg::GetOptions(bool *use_notify, bool *use_mailbox) {
  *use_notify = false;
  *use_mailbox = false;
  for (auto *reader : readers_) {
    if (resource::IsDataFlowIn(*(reader->GetClass()))) {
      *use_notify |= true;
    } else {
      bool n, m;
      SharedRegAccessor::GetAccessorFeatures(reader, &n, &m);
      *use_notify |= n;
      *use_mailbox |= m;
    }
  }
  for (auto *writer : writers_) {
    bool n, m;
    SharedRegAccessor::GetAccessorFeatures(writer, &n, &m);
    *use_notify |= n;
    *use_mailbox |= m;
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
