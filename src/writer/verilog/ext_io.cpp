#include "writer/verilog/ext_io.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/connection.h"
#include "writer/module_template.h"
#include "writer/names.h"
#include "writer/verilog/module.h"
#include "writer/verilog/port.h"
#include "writer/verilog/state.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/accessor_info.h"
#include "writer/verilog/wire/wire_set.h"

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
  if (resource::IsExtInput(*klass)) {
    BuildExtInputResource();
  }
  if (resource::IsExtOutput(*klass)) {
    BuildExtOutputResource();
  }
}

void ExtIO::BuildExtInputResource() {
  auto *params = res_.GetParams();
  string input_port;
  int width;
  params->GetExtInputPort(&input_port, &width);
  AddPortToTop(input_port, false, false, width);
  BuildBufRegChain(input_port, width);
  string input_src = InputRegName(res_);
  if (distance_ > 0) {
    ostream &ss = tab_.StateOutputSectionStream();
    ss << "      " << input_src
       << " <= "
       << input_port << ";\n";
  }
  auto &conn = tab_.GetModule()->GetConnection();
  const vector<IResource *> &acs = conn.GetExtInputAccessors(&res_);
  if (acs.size() > 0) {
    wire::WireSet *ws = new wire::WireSet(*this, input_src);
    for (auto *ac : acs) {
      wire::AccessorInfo *ainfo = ws->AddAccessor(ac);
      ainfo->SetDistance(ac->GetParams()->GetDistance());
      ainfo->AddSignal("r", wire::AccessorSignalType::ACCESSOR_READ_ARG, width);
    }
    ws->Build();
  }
}

void ExtIO::BuildExtOutputResource() {
  auto *params = res_.GetParams();
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
  BuildBufRegChain(output_port, width);
  if (distance_ > 0) {
    ss << "      " << output_port << " <= "
       << BufRegName(output_port, 0);
    ss << ";\n";
  }
}

void ExtIO::BuildBufRegChain(const string &port, int width) {
  // The data flows as follows.
  //   port0ofN <= port1ofN
  //   port1ofN <= port2ofN
  //   ..
  //   portN-2ofN <= portN-2ofN
  ostream &rs = tab_.ResourceSectionStream();
  ostream &is = tab_.InitialValueSectionStream();
  for (int i = 0; i < distance_; ++i) {
    rs << "  reg " << Table::WidthSpec(width)
       << BufRegName(port, i) << ";\n";
    is << "      " << BufRegName(port, i) << " <= 0;\n";
  }
  ostream &ss = tab_.StateOutputSectionStream();
  for (int i = 1; i < distance_; ++i) {
    ss << "      " << BufRegName(port, i - 1) << " <= "
       << BufRegName(port, i);
    ss << ";\n";
  }
}

void ExtIO::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsExtInput(*klass)) {
    BuildExtInputInsn(insn);
  }
  if (resource::IsExtOutput(*klass)) {
    BuildExtOutputInsn(insn, st);
  }
}

void ExtIO::BuildExtInputInsn(IInsn *insn) {
  auto *params = res_.GetParams();
  string input_port;
  int width;
  params->GetExtInputPort(&input_port, &width);
  ostream &ws = tab_.InsnWireValueSectionStream();
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
     << " = ";
  if (distance_ > 0) {
    ws << BufRegName(input_port, 0) << ";\n";
  } else {
    ws << input_port << ";\n";
  }
}

void ExtIO::BuildExtOutputInsn(IInsn *insn, State *st) {
  if (insn->outputs_.size() > 0) {
    BuildPeekExtOutputInsn(insn);
    return;
  }
  if (has_default_output_value_) {
    // The code is generated in BuildResource()
    return;
  }
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
  return BufRegNameWithDistance(output_port, distance_, stage);
}

void ExtIO::BuildPeekExtOutputInsn(IInsn *insn) {
  auto *params = res_.GetParams();
  ostream &ws = tab_.InsnWireValueSectionStream();
  string output_port;
  int width;
  params->GetExtOutputPort(&output_port, &width);
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
     << "  = ";
  if (distance_ > 0) {
    ws << BufRegName(output_port, distance_ - 1);
  } else {
    ws << output_port;
  }
  ws << ";\n";
}

string ExtIO::InputRegName(const IResource &res) {
  auto *params = res.GetParams();
  string input_port;
  int width;
  params->GetExtInputPort(&input_port, &width);
  int distance = params->GetDistance();
  if (distance == 0) {
    return input_port;
  }
  return BufRegNameWithDistance(input_port, distance, distance - 1);
}

string ExtIO::BufRegNameWithDistance(const string &port,
				     int distance, int stage) {
  return port + "_buf" + Util::Itoa(stage) + "of" + Util::Itoa(distance);
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
