#include "writer/verilog/channel.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/module_template.h"
#include "writer/verilog/module.h"
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
  }
}

void Channel::BuildInsn(IInsn *insn, State *st) {
}

string Channel::DataPort(const IChannel &ic) {
  if (ic.GetReader() != nullptr) {
      if (ic.GetWriter() != nullptr) {
	return "channel_data_" + Util::Itoa(ic.GetId());
      } else {
	return "ext_rdata_" + Util::Itoa(ic.GetId());
      }
  } else {
    return "ext_wdata_" + Util::Itoa(ic.GetId());
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
