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
    readers_(nullptr), writers_(nullptr),
    need_write_arbitration_(false) {
  auto *klass = res_.GetClass();
  CHECK(resource::IsSharedReg(*klass));
  auto *params = res_.GetParams();
  string unused;
  params->GetExtOutputPort(&unused, &width_);
  has_default_output_value_ =
    params->GetDefaultValue(&default_output_value_);
  readers_ = table.GetModule()->GetConnection().GetSharedRegReaders(&res_);
  writers_ = table.GetModule()->GetConnection().GetSharedRegWriters(&res_);
  if (writers_ != nullptr || has_default_output_value_) {
    need_write_arbitration_ = true;
  }
  GetOptions(&use_notify_, &use_sem_);
}

void SharedReg::BuildResource() {
  ostream &rs = tmpl_->GetStream(kRegisterSection);
  ostream &is = tab_.InitialValueSectionStream();
  rs << "  // shared-reg";
  if (use_notify_) {
    rs << " use-notify";
  }
  if (use_sem_) {
    rs << " use-sem";
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
    for (auto *writer : *writers_) {
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
  if (use_sem_) {
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
    if (writers_ != nullptr) {
      for (auto *res : *writers_) {
	value = WriterEnName(*res) + " ? " + WriterName(*res) + " : (" + value + ")";
      }
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
  vector<string> higher_put_reqs;
  vector<string> put_reqs;
  for (auto *writer : *writers_) {
    if (!SharedRegAccessor::UseMailbox(writer)) {
      continue;
    }
    rs << "  wire " << RegMailboxPutAckName(*writer) << ";\n";
    rs << "  assign " << RegMailboxPutAckName(*writer) << " = "
       << "(!" << RegMailboxName(res_) << ") && ";
    if (higher_put_reqs.size()) {
      rs << "(!(" << Util::Join(higher_put_reqs, " | ") << ")) && ";
    }
    rs << RegMailboxPutReqName(*writer) << ";\n";
    put_reqs.push_back(RegMailboxPutReqName(*writer));
  }
  vector<string> higher_get_reqs;
  vector<string> get_reqs;
  for (auto *reader : *readers_) {
    if (!SharedRegAccessor::UseMailbox(reader)) {
      continue;
    }
    rs << "  wire " << RegMailboxGetAckName(*reader) << ";\n";
    rs << "  assign " << RegMailboxGetAckName(*reader) << " = "
       << "(" << RegMailboxName(res_) << ") && ";
    if (higher_get_reqs.size()) {
      rs << "(!(" << Util::Join(higher_get_reqs, " | ") << ")) && ";
    }
    rs << RegMailboxGetReqName(*reader) << ";\n";
    get_reqs.push_back(RegMailboxGetReqName(*reader));
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

string SharedReg::WriterName(const IResource &res) {
  return RegName(res) + "_w";
}

string SharedReg::WriterEnName(const IResource &res) {
  return WriterName(res) + "_en";
}

string SharedReg::RegNotifierName(const IResource &res) {
  return RegName(res) + "_notify";
}

string SharedReg::WriterNotifierName(const IResource &res) {
  return RegName(res) + "_write_notify";
}

string SharedReg::RegMailboxName(const IResource &res) {
  return RegName(res) + "_mailbox";
}

string SharedReg::RegMailboxPutReqName(const IResource &res) {
  return RegName(res) + "_mailbox_put_req";
}

string SharedReg::RegMailboxPutAckName(const IResource &res) {
  return RegName(res) + "_mailbox_put_ack";
}

string SharedReg::RegMailboxGetReqName(const IResource &res) {
  return RegName(res) + "_mailbox_get_req";
}

string SharedReg::RegMailboxGetAckName(const IResource &res) {
  return RegName(res) + "_mailbox_get_ack";
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
  if (readers_ == nullptr) {
    return;
  }
  IModule *reg_module = res_.GetTable()->GetModule();
  set<const IModule *> wired_modules;
  set<const IModule *> has_upward;
  set<const IModule *> has_downward;
  for (auto *reader : *readers_) {
    IModule *reader_module = reader->GetTable()->GetModule();
    const IModule *common_root = Connection::GetCommonRoot(reg_module,
							   reader_module);
    if (reader_module != common_root && reg_module != common_root) {
      if (wired_modules.find(common_root) == wired_modules.end()) {
	AddWire(common_root, &tab_, &res_, false);
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

void SharedReg::AddWire(const IModule *common_root, const Table *tab,
			const IResource *accessor, bool is_write) {
  Module *mod = tab->GetModule()->GetByIModule(common_root);
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
  bool notify = SharedRegAccessor::UseNotify(accessor);
  if (notify) {
    rs << "  wire ";
    if (is_write) {
      rs << WriterNotifierName(*accessor) << ";\n";
    } else {
      rs << RegNotifierName(*accessor) << ";\n";
    }
  }
  bool sem = SharedRegAccessor::UseMailbox(accessor);
  if (sem) {
    if (is_write) {
      rs << "  wire "
	 << RegMailboxPutReqName(*accessor) << ";\n"
	 << "  wire "
	 << RegMailboxPutAckName(*accessor) << ";\n";
    } else {
      rs << "  wire "
	 << RegMailboxGetReqName(*accessor) << ";\n"
	 << "  wire "
	 << RegMailboxGetAckName(*accessor) << ";\n";
    }
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

void SharedReg::GetOptions(bool *use_notify, bool *use_sem) {
  *use_notify = false;
  *use_sem = false;
  if (readers_ == nullptr || writers_ == nullptr) {
    return;
  }
  for (auto *reader : *readers_) {
    bool n, s;
    SharedRegAccessor::GetAccessorFeatures(reader, &n, &s);
    *use_notify |= n;
    *use_sem |= s;
  }
  for (auto *writer : *writers_) {
    bool n, s;
    SharedRegAccessor::GetAccessorFeatures(writer, &n, &s);
    *use_notify |= n;
    *use_sem |= s;
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
