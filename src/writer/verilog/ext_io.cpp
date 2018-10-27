#include "writer/verilog/ext_io.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/names.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/state.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

ExtIO::ExtIO(const IResource &res, const Table &table)
  : Resource(res, table), has_default_output_value_(false),
    default_output_value_(0) {
  auto *klass = res.GetClass();
  auto *params = res.GetParams();
  if (resource::IsExtOutput(*klass)) {
    has_default_output_value_ =
      params->GetDefaultValue(&default_output_value_);
  }
  distance_ = params->GetDistance();
}

void ExtIO::BuildResource() {
  auto *klass = res_.GetClass();
  auto *params = res_.GetParams();
  if (resource::IsExtInput(*klass)) {
    string input_port;
    int width;
    params->GetExtInputPort(&input_port, &width);
    AddPortToTop(input_port, false, false, width);
  }
  if (resource::IsExtOutput(*klass)) {
    string output_port;
    int width;
    params->GetExtOutputPort(&output_port, &width);
    ostream &ss = tab_.StateOutputSectionStream();
    if (has_default_output_value_) {
      ss << "      ";
      if (distance_ == 0) {
	ss << output_port;
      } else {
	ss << BufRegName(output_port, distance_ - 1);
      }
      ss << " <= "
	 << SelectValueByState(Util::Itoa(default_output_value_)) << ";\n";
    }
    AddPortToTop(output_port, true, false, width);
    ostream &rs = tab_.ResourceSectionStream();
    ostream &is = tab_.InitialValueSectionStream();
    for (int i = 0; i < distance_; ++i) {
      rs << "  reg " << Table::WidthSpec(width)
	 << BufRegName(output_port, i) << ";\n";
      is << "      " << BufRegName(output_port, i) << " <= 0;\n";
    }
    if (distance_ > 0) {
      ss << "      " << output_port << " <= "
	 << BufRegName(output_port, 0);
      ss << ";\n";
      for (int i = 1; i < distance_; ++i) {
	ss << "      " << BufRegName(output_port, i - 1) << " <= "
	   << BufRegName(output_port, i);
	ss << ";\n";
      }
    }
  }
}

void ExtIO::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsExtInput(*klass)) {
    BuildExtInputInsn(insn);
  }
  if (resource::IsExtOutput(*klass) &&
      !has_default_output_value_) {
    auto *params = res_.GetParams();
    string output_port;
    int width;
    params->GetExtOutputPort(&output_port, &width);
    ostream &os = st->StateTransitionSectionStream();
    if (distance_ == 0) {
      os << "      " << output_port;
    } else {
      os << "          " << BufRegName(output_port, distance_ - 1);
    }
    os << " <= "
       << InsnWriter::RegisterValue(*insn->inputs_[0], tab_.GetNames());
    os << ";\n";
  }
}

void ExtIO::BuildExtInputInsn(IInsn *insn) {
  auto *params = res_.GetParams();
  string input_port;
  int width;
  params->GetExtInputPort(&input_port, &width);
  ostream &ws = tab_.InsnWireValueSectionStream();
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
     << " = "
     << input_port << ";\n";
}

void ExtIO::CollectNames(Names *names) {
  auto *klass = res_.GetClass();
  auto *params = res_.GetParams();
  string port;
  int width;
  if (resource::IsExtInput(*klass)) {
    params->GetExtInputPort(&port, &width);
  } else if (resource::IsExtOutput(*klass)) {
    params->GetExtOutputPort(&port, &width);
  }
  names->ReserveGlobalName(port);
}

string ExtIO::BufRegName(const string &output_port, int stage) {
  return output_port + "_buf" + Util::Itoa(stage) + "of" + Util::Itoa(distance_);
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
