#include "writer/verilog/table.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "iroha/stl_util.h"
#include "writer/module_template.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/state.h"

namespace iroha {
namespace verilog {

Table::Table(ITable *table, Ports *ports, Module *mod, Embed *embed,
	     ModuleTemplate *tmpl, int nth)
  : i_table_(table), ports_(ports), mod_(mod), embed_(embed),
    tmpl_(tmpl), nth_(nth) {
  st_ = "st_" + Util::Itoa(nth);
}

Table::~Table() {
  STLDeleteValues(&states_);
}

void Table::Build() {
  for (auto *i_state : i_table_->states_) {
    State *st = new State(i_state, this);
    st->Build();
    states_.push_back(st);
  }

  BuildStateDecl();
  BuildRegister();
  BuildResource();
}

void Table::BuildStateDecl() {
  ostream &sd = tmpl_->GetStream(kStateDeclSection);
  int max_id = 0;
  for (auto *st : i_table_->states_) {
    int id = st->GetId();
    sd << "  `define " << StateName(id) << " "
       << id << "\n";
    if (id > max_id) {
      max_id = id;
    }
  }
  int bits = 0;
  int u = 1;
  while (u < max_id) {
    u *= 2;
    ++bits;
  }
  sd << "\n";

  ostream &sv = tmpl_->GetStream(kStateDeclSection);
  sv << "  reg [" << bits << ":0] " << StateVariable() << ";\n";
}

void Table::BuildResource() {
  for (auto *res : i_table_->resources_) {
    auto *klass = res->GetClass();
    auto *params = res->GetParams();
    if (klass->GetName() == resource::kExtInput) {
      string input_port;
      int width;
      params->GetExtInputPort(&input_port, &width);
      ports_->AddPort(input_port, Port::INPUT, width);
    }
    if (klass->GetName() == resource::kExtOutput) {
      string output_port;
      int width;
      params->GetExtOutputPort(&output_port, &width);
      ports_->AddPort(output_port, Port::OUTPUT, width);
    }
    if (klass->GetName() == resource::kEmbedded) {
      BuildEmbededResource(*res);
    }
    if (resource::IsBinOp(*klass)) {
      BuildBinOpResource(*res);
    }
    if (resource::IsArray(*klass)) {
      BuildArrayResource(*res);
    }
  }
}

void Table::BuildBinOpResource(const IResource &res) {
  ostream &rs = tmpl_->GetStream(kResourceSection);
  const string &res_name = res.GetClass()->GetName();
  rs << "  // " << res_name << ":" << res.GetId() << "\n";
  map<IState *, IInsn *> callers;
  CollectResourceCallers(res, &callers);
  if (callers.size() == 0) {
    return;
  }
  string name = InsnWriter::ResourceName(res);
  WriteInputSel(name + "_s0", res, callers, 0, rs);
  WriteInputSel(name + "_s1", res, callers, 1, rs);
  WriteWire(name + "_d0", res.output_types_[0], rs);

  rs << "  assign " << name << + "_d0 = "
     << name + "_s0 ";
  if (res_name == resource::kGt) {
    rs << ">";
  } else if (res_name == resource::kAdd) {
    rs << "+";
  } else {
    LOG(FATAL) << "Unknown binop" << res_name;
  }
  rs << " " << name + "_s1;\n";
}

void Table::BuildArrayResource(const IResource &res) {
}

void Table::BuildEmbededResource(const IResource &res) {
  auto *params = res.GetParams();
  embed_->RequestModule(*params);
  ostream &is = tmpl_->GetStream(kEmbeddedInstanceSection);
  embed_->BuildModuleInstantiation(res, *ports_, is);
}

void Table::BuildRegister() {
  ostream &rs = tmpl_->GetStream(kRegisterSection);
  ostream &is = tmpl_->GetStream(kInitialValueSection + Util::Itoa(nth_));
  for (auto *reg : i_table_->registers_) {
    if (!reg->IsConst() && !reg->IsStateLocal()) {
      rs << "  reg";
      if (reg->value_type_.GetWidth() > 0) {
	rs << " [" << reg->value_type_.GetWidth() << ":0]";
      }
      rs << " " << reg->GetName() << ";\n";
    }
    if (!reg->IsConst() && reg->HasInitialValue()) {
      is << "      " << reg->GetName() << " <= "
	 << reg->GetInitialValue().value_ << ";\n";
    }
  }
}

void Table::Write(ostream &os) {
  os << "  always @(posedge " << ports_->GetClk() << ") begin\n";
  os << "    if (";
  if (!mod_->GetResetPolarity()) {
    os << "!";
  }
  os << ports_->GetReset() << ") begin\n";
  IState *st = i_table_->GetInitialState();
  if (st == nullptr) {
    LOG(FATAL) << "null initial state.\n";
  }
  os << "      " << StateVariable() << " <= `"
     << StateName(st->GetId()) << ";\n";
  os << tmpl_->GetContents(kInitialValueSection + Util::Itoa(nth_));
  os << "    end else begin\n";
  os << "      case (" << StateVariable() << ")\n";
  for (auto *state : states_) {
    state->Write(os);
  }
  os << "      endcase\n";
  os << "    end\n";
  os << "  end\n";
}

const string &Table::StateVariable() const {
  return st_;
}

string Table::StateName(int id) {
  return "S_" + Util::Itoa(nth_) + "_" + Util::Itoa(id);
}

ModuleTemplate *Table::GetModuleTemplate() const {
  return tmpl_;
}

void Table::CollectResourceCallers(const IResource &res,
				   map<IState *, IInsn *> *callers) {
  for (auto *st : i_table_->states_) {
    for (auto *insn : st->insns_) {
      if (insn->GetResource() == &res) {
	callers->insert(make_pair(st, insn));
      }
    }
  }
}

void Table::WriteWire(const string &name, const IValueType &type,
		      ostream &os) {
  os << "  wire ";
  int width = type.GetWidth();
  if (width > 0) {
    os << "[" << (width - 1) << ":0] ";
  }
  os << name << ";\n";
}

void Table::WriteInputSel(const string &name, const IResource &res,
			  const map<IState *, IInsn *> &callers,
			  int nth,
			  ostream &os) {
  WriteWire(name, res.input_types_[nth], os);
  os << "  assign " << name << " = ";
  if (callers.size() == 1) {
    IInsn *insn = (callers.begin())->second;
    os << InsnWriter::RegisterName(*insn->inputs_[nth]);
  } else {
    LOG(FATAL) << "TODO(yt76): Input selector";
  }
  os << ";\n";
}

}  // namespace verilog
}  // namespace iroha

