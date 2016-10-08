#include "writer/verilog/port_io.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

PortIO::PortIO(const IResource &res, const Table &table)
  : Resource(res, table), has_default_output_value_(false),
    default_output_value_(0) {
  auto *klass = res_.GetClass();
  if (resource::IsPortOutput(*klass)) {
    output_port_ = PortName(res);
    auto *params = res_.GetParams();
    string unused;
    params->GetExtOutputPort(&unused, &width_);
    has_default_output_value_ =
      params->GetDefaultValue(&default_output_value_);
  } else {
    width_ = 0;
  }
}

void PortIO::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsPortOutput(*klass)) {
    ostream &rs = tmpl_->GetStream(kRegisterSection);
    rs << "  // port-output\n";
    rs << "  reg ";
    if (width_ > 0) {
      rs << "[" << width_ -1 << ":0]";
    }
    rs << " " << output_port_ << ";\n";
    if (has_default_output_value_) {
      ostream &os = tab_.StateOutputSectionStream();
      os << "      " << output_port_ << " <= "
	 << SelectValueByState(default_output_value_) << ";\n";
    }
  }
}

void PortIO::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsPortOutput(*klass) &&
      !has_default_output_value_) {
    ostream &os = st->StateBodySectionStream();
    os << "          " << output_port_ << " <= "
       << InsnWriter::RegisterName(*insn->inputs_[0]);
    os << ";\n";
  }
  if (resource::IsPortInput(*klass)) {
    IResource *input = res_.GetPortInput();
    CHECK(input->GetTable()->GetModule()->GetId() ==
	  res_.GetTable()->GetModule()->GetId())
      << "port-input from different module isn't yet supported";
    ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
    ws << "  assign "
       << InsnWriter::InsnOutputWireName(*insn, 0)
       << " = "
       << PortName(*input) << ";\n";
  }
}

string PortIO::PortName(const IResource &res) {
  auto *params = res.GetParams();
  int unused_width;
  string port_name;
  params->GetExtOutputPort(&port_name, &unused_width);
  ITable *tab = res.GetTable();
  IModule *mod = tab->GetModule();
  return "port_output_" +
    Util::Itoa(mod->GetId()) + "_" +
    Util::Itoa(tab->GetId()) + "_" + Util::Itoa(res.GetId()) +
    "_" + port_name;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
