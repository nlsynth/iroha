#include "writer/verilog/ext_io.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/state.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

ExtIO::ExtIO(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void ExtIO::BuildResource() {
  auto *klass = res_.GetClass();
  auto *params = res_.GetParams();
  if (resource::IsExtInput(*klass)) {
    auto *ports = tab_.GetPorts();
    string input_port;
    int width;
    params->GetExtInputPort(&input_port, &width);
    ports->AddPort(input_port, Port::INPUT, width);
  }
  if (resource::IsExtOutput(*klass)) {
    auto *ports = tab_.GetPorts();
    string output_port;
    int width;
    params->GetExtOutputPort(&output_port, &width);
    ports->AddPort(output_port, Port::OUTPUT, width);
  }
}

void ExtIO::BuildInsn(IInsn *insn, State *st) {
  auto *klass = res_.GetClass();
  if (resource::IsExtInput(*klass)) {
    BuildExtInputInsn(insn);
  }

  ostream &os = st->StateBodySectionStream();
  InsnWriter writer(insn, st, os);
  if (resource::IsExtOutput(*klass)) {
    writer.ExtOutput();
  }
}

void ExtIO::BuildExtInputInsn(IInsn *insn) {
  auto *params = res_.GetParams();
  string input_port;
  int width;
  params->GetExtInputPort(&input_port, &width);
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, 0)
     << " = "
     << input_port << ";\n";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
