#include "writer/verilog/shared_reg_ext_writer.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "writer/verilog/shared_reg.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/names.h"

namespace iroha {
namespace writer {
namespace verilog {

SharedRegExtWriter::SharedRegExtWriter(const IResource &res,
				       const Table &table)
  : Resource(res, table) {
}

void SharedRegExtWriter::BuildResource() {
  int width =  res_.GetParentResource()->GetParams()->GetWidth();
  string wrn = SharedReg::GetNameRW(*(res_.GetParentResource()),
				    true);
  string w_wire = wire::Names::AccessorWire(wrn, &res_, "w");
  string wen_wire = wire::Names::AccessorWire(wrn, &res_, "wen");
  string notify_wire = wire::Names::AccessorWire(wrn, &res_, "notify");
  string put_wire = wire::Names::AccessorWire(wrn, &res_, "put_req");
  string put_ack_wire = wire::Names::AccessorWire(wrn, &res_, "put_ack");
  string w;
  string wen;
  auto *params = res_.GetParams();
  string raw_name = params->GetPortNamePrefix();
  string name = raw_name;
  if (name.empty()) {
    w = wire::Names::AccessorSignalBase(wrn, &res_, "w");
    wen = wire::Names::AccessorSignalBase(wrn, &res_, "wen");
  } else {
    w = name;
    string s = params->GetWenSuffix();
    if (s.empty()) {
      s = "wen";
    }
    wen = name + "_" + s;
  }
  AddPortToTop(w, false, true, width);
  AddPortToTop(wen, false, true, 0);
  string notify = params->GetNotifySuffix();
  if (!notify.empty()) {
    notify = raw_name + "_" + notify;
    AddPortToTop(notify, false, true, 0);
  }
  string put = params->GetPutSuffix();
  string put_ack;
  if (!put.empty()) {
    put_ack = raw_name + "_" + put + "_ack";
    put = raw_name + "_" + put;
    AddPortToTop(put, false, true, 0);
    AddPortToTop(put_ack, true, true, 0);
  }
  ostream &rvs = tab_.ResourceValueSectionStream();
  rvs << "  // shared-reg-ext-writer\n"
      << "  assign " << w_wire << " = " << w << ";\n"
      << "  assign " << wen_wire << " = " << wen << ";\n";
  if (!notify.empty()) {
    rvs << "  assign " << notify_wire << " = " << notify << ";\n";
  }
  if (!put.empty()) {
    rvs << "  assign " << put_wire << " = " << put << ";\n";
    rvs << "  assign " << put_ack << " = " << put_ack_wire << ";\n";
  }
}

void SharedRegExtWriter::BuildInsn(IInsn *insn, State *st) {
}

void SharedRegExtWriter::GetAccessorFeatures(const IResource *accessor,
					     bool *use_notify,
					     bool *use_mailbox) {
  *use_notify = UseNotify(accessor);
  *use_mailbox = UseMailbox(accessor);
}

bool SharedRegExtWriter::UseNotify(const IResource *accessor) {
  auto *params = accessor->GetParams();
  return !params->GetNotifySuffix().empty();
}

bool SharedRegExtWriter::UseMailbox(const IResource *accessor) {
  auto *params = accessor->GetParams();
  return !params->GetPutSuffix().empty();
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
