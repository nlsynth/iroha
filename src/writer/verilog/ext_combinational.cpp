// ext-combinational resource wraps an external combinational logic.
// This assigns a set of input values and gets the outputs from the circuit
// within the cycle.
#include "writer/verilog/ext_combinational.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"
#include "writer/module_template.h"
#include "writer/verilog/embedded_modules.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/port.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

ExtCombinational::ExtCombinational(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void ExtCombinational::BuildResource() {
  string connection;
  for (int i = 0; i < res_.input_types_.size(); ++i) {
    AddPort(ArgPin(&res_, i), ArgPin(nullptr, i),
	    res_.input_types_[i].GetWidth(), &connection);
  }
  for (int i = 0; i < res_.output_types_.size(); ++i) {
    AddPort(RetPin(&res_, i), RetPin(nullptr, i),
	    res_.output_types_[i].GetWidth(), &connection);
  }
  BuildEmbeddedModule(connection);

  map<IState *, IInsn *> callers;
  CollectResourceCallers("", &callers);
  if (callers.size() == 0) {
    return;
  }
  ostream &rs = tab_.ResourceSectionStream();
  string name = InsnWriter::CustomResourceName("ext_combinational", res_);
  for (int i = 0; i < res_.input_types_.size(); ++i) {
    string s = name + "_s" + Util::Itoa(i);
    WriteInputSel(s, callers, i, rs);
    rs << "  assign " << ArgPin(&res_, i) << " = " << s << ";\n";
  }
  for (int i = 0; i < res_.output_types_.size(); ++i) {
    string d = name + "_d" + Util::Itoa(i);
    WriteWire(d, res_.output_types_[i], rs);
    rs << "  assign " << d << " = " << RetPin(&res_, i) << ";\n";
  }
}

void ExtCombinational::BuildInsn(IInsn *insn, State *st) {
  ostream &ws = tab_.InsnWireValueSectionStream();
  for (int i = 0; i < res_.output_types_.size(); ++i) {
    ws << "  assign " << InsnWriter::InsnOutputWireName(*insn, i)
       << " = " << InsnWriter::CustomResourceName("ext_combinational", res_)
       << "_d" << i << ";\n";
  }
}

void ExtCombinational::CollectNames(Names *names) {
}

void ExtCombinational::AddPort(const string &name, const string &wire_name,
			       int width,
			       string *connection) {
  ostream &rs = tab_.ResourceSectionStream();
  rs << "  wire " << Table::WidthSpec(width) << name << ";\n";
  *connection += ", ." + wire_name + "(" + name + ")";
}

string ExtCombinational::ArgPin(const IResource *res, int nth) {
  return PinName(res, "arg", nth);
}

string ExtCombinational::RetPin(const IResource *res, int nth) {
  return PinName(res, "ret", nth);
}

string ExtCombinational::PinName(const IResource *res, const string &name, int nth) {
  string n = name;
  if (res != nullptr) {
    const string &prefix = res->GetParams()->GetExtTaskName();
    if (!prefix.empty()) {
      n = prefix + "_" + n;
    }
    n += "_" + Util::Itoa(res->GetTable()->GetId()) + "_" + Util::Itoa(nth);
    n += "_" + Util::Itoa(res->GetId());
  }
  n += "_" + Util::Itoa(nth);
  return n;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
