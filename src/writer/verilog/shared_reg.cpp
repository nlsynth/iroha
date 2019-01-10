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
#include "writer/verilog/shared_reg_accessor.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/inter_module_wire.h"

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
  ostream &rs = tab_.ResourceSectionStream();
  ostream &is = tab_.InitialValueSectionStream();
  rs << "  // shared-reg";
  if (use_notify_) {
    rs << " use-notify";
  }
  if (use_mailbox_) {
    rs << " use-mailbox";
  }
  rs << "\n";
  if (readers_.size() == 0) {
    rs << "  reg ";
    if (width_ > 0) {
      rs << "[" << width_ - 1 << ":0]";
    }
    rs << " " << RegName(res_) << ";\n";
  }
  // Reset value
  is << "      " << RegName(res_) << " <= ";
  if (has_default_output_value_) {
    is << default_output_value_;
  } else {
    is << 0;
  }
  is << ";\n";
  if (use_notify_) {
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
	en = "(" + en + " || " + RegMailboxPutAckName(*writer) + ")";
      }
      value = en + " ? " + WriterName(*writer) + " : (" + value + ")";
    }
    os << SelectValueByState(value);
    os << ";\n";
  }
  BuildAccessorWire();
}

void SharedReg::BuildMailbox() {
  ostream &rs = tab_.ResourceSectionStream();
  rs << "  reg " << RegMailboxName(res_) << ";\n";
  ostream &rvs = tab_.ResourceValueSectionStream();
  ostream &is = tab_.InitialValueSectionStream();
  is << "      " << RegMailboxName(res_) << " <= 0;\n";
  vector<string> put_reqs;
  if (writers_.size() > 0) {
    for (auto *writer : writers_) {
      if (!SharedRegAccessor::UseMailbox(writer)) {
	continue;
      }
      rvs << "  assign " << RegMailboxPutAckName(*writer) << " = "
	  << "(!" << RegMailboxName(res_) << ") && ";
      if (put_reqs.size() > 0) {
	rvs << "(!(" << Util::Join(put_reqs, " | ") << ")) && ";
      }
      rvs << RegMailboxPutReqName(*writer) << ";\n";
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
      rvs << "  assign " << RegMailboxGetAckName(*reader) << " = "
	 << "(" << RegMailboxName(res_) << ") && ";
      if (get_reqs.size() > 0) {
	rvs << "(!(" << Util::Join(get_reqs, " | ") << ")) && ";
      }
      rvs << RegMailboxGetReqName(*reader) << ";\n";
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
      ostream &ws = tab_.InsnWireValueSectionStream();
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

string SharedReg::RegMailboxBufName(const IResource &reader) {
  return RegName(reader) + "_mailbox_buf";
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

void SharedReg::BuildAccessorWire() {
  wire::InterModuleWire wire(*this);
  int dw = res_.GetParams()->GetWidth();
  auto &conn = tab_.GetModule()->GetConnection();
  auto &readers = conn.GetSharedRegReaders(&res_);
  for (auto *reader : readers) {
    wire.AddWire(*reader, RegName(res_), dw, true, true);
    if (SharedRegAccessor::UseNotify(reader)) {
      wire.AddWire(*reader, RegNotifierName(res_), 0, true, true);
    }
    if (SharedRegAccessor::UseMailbox(reader)) {
      wire.AddWire(*reader, RegMailboxGetReqName(*reader), 0, false, false);
      wire.AddWire(*reader, RegMailboxGetAckName(*reader), 0, true, false);
    }
  }
  // TODO: Move all writer wiring logic to here too.
  auto &writers = conn.GetSharedRegWriters(&res_);
  for (auto *writer : writers) {
    wire.AddWire(*writer, WriterName(*writer), dw, false, false);
    wire.AddWire(*writer, WriterEnName(*writer), 0, false, false);
    if (SharedRegAccessor::UseNotify(writer)) {
      wire.AddWire(*writer, WriterNotifierName(*writer), 0, false, false);
    }
    if (SharedRegAccessor::UseMailbox(writer)) {
      wire.AddWire(*writer, RegMailboxPutReqName(*writer), 0, true, false);
      wire.AddWire(*writer, RegMailboxPutAckName(*writer), 0, false, false);
    }
  }
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
