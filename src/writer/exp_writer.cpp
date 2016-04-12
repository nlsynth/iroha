#include "writer/exp_writer.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"

namespace iroha {

ExpWriter::ExpWriter(const IDesign *design, ostream &os)
  : design_(design), os_(os) {
}

void ExpWriter::Write() {
  WriteResourceParams(*design_->GetParams(), "");
  for (auto *mod : design_->modules_) {
    WriteModule(*mod);
  }
}

void ExpWriter::WriteModule(const IModule &mod) {
  os_ << "(MODULE " << mod.GetName() << "\n";
  IModule *parent = mod.GetParentModule();
  if (parent != nullptr) {
    os_ << "  (PARENT " << parent->GetName() << ")\n";
  }
  WriteResourceParams(*mod.GetParams(), "  ");
  for (auto *tab : mod.tables_) {
    WriteTable(*tab);
  }
  os_ << ")\n";
}

void ExpWriter::WriteTable(const ITable &tab) {
  os_ << "  (TABLE " << tab.GetId() << "\n";
  WriteRegisters(tab);
  WriteResources(tab);
  WriteInitialState(tab);
  for (auto *st : tab.states_) {
    WriteState(*st);
  }
  os_ << "  )\n";
}

void ExpWriter::WriteResources(const ITable &tab) {
  os_ << "    (RESOURCES\n";
  for (auto *res : tab.resources_) {
    WriteResource(*res);
  }
  os_ << "    )\n";
}

void ExpWriter::WriteResource(const IResource &res) {
  const IResourceClass &rc = *(res.GetClass());
  os_ << "      (RESOURCE " << res.GetId()
      << " " << res.GetClass()->GetName() << "\n";
  os_ << "        ";
  WriteResourceTypes(res.input_types_);
  os_ << " ";
  WriteResourceTypes(res.output_types_);
  os_ << "\n";
  ResourceParams *params = res.GetParams();
  WriteResourceParams(*params, "       ");
  if (resource::IsArray(rc) || res.GetArray() != nullptr) {
    WriteArrayDesc(res);
  }
  if (resource::IsSubModuleTaskCall(rc)) {
    WriteSubModuleTaskCall(res);
  }
  os_ << "      )\n";
}

void ExpWriter::WriteArrayDesc(const IResource &res) {
  const IArray *array = res.GetArray();
  os_ << "        (ARRAY ";
  os_ << array->GetAddressWidth();
  os_ << " ";
  WriteValueType(array->GetDataType());
  if (array->IsExternal()) {
    os_ << " EXTERNAL";
  } else {
    os_ << " INTERNAL";
  }
  if (array->IsRam()) {
    os_ << " RAM";
  } else {
    os_ << " ROM";
  }
  os_ << ")\n";
}

void ExpWriter::WriteSubModuleTaskCall(const IResource &res) {
  const ITable *table = res.GetCalleeTable();
  const IModule *mod = table->GetModule();
  os_ << "        (MODULE " << mod->GetName() << " " << table->GetId() << ")\n";
}

void ExpWriter::WriteInitialState(const ITable &tab) {
  IState *st = tab.GetInitialState();
  if (st) {
    os_ << "    (INITIAL " << st->GetId() << ")\n";
  }
}

void ExpWriter::WriteRegisters(const ITable &tab) {
  os_ << "    (REGISTERS\n";
  for (auto *reg : tab.registers_) {
    os_ << "      (REGISTER " << reg->GetId() << " ";
    const string &name = reg->GetName();
    if (name.empty()) {
      os_ << "()";
    } else {
      os_ << name;
    }
    os_ << "\n";
    os_ << "        ";
    if (reg->IsConst()) {
      os_ << "CONST";
    } else if (reg->IsStateLocal()) {
      os_ << "WIRE";
    } else {
      os_ << "REG";
    }
    os_ << " ";
    WriteValueType(reg->value_type_);
    os_ << " ";
    if (reg->HasInitialValue()) {
      WriteValue(reg->GetInitialValue());
    } else {
      os_ << "()";
    }
    os_ << "\n";
    os_ << "      )\n";
  }
  os_ << "    )\n";
}

void ExpWriter::WriteValueType(const IValueType &type) {
  os_ << "UINT " << type.GetWidth();
}

void ExpWriter::WriteValue(const IValue &value) {
  os_ << value.value_;
}

void ExpWriter::WriteState(const IState &st) {
  os_ << "    (STATE " << st.GetId() << "\n";
  for (auto *insn : st.insns_) {
    WriteInsn(*insn);
  }
  os_ << "    )\n";
}

void ExpWriter::WriteInsn(const IInsn &insn) {
  os_ << "      (INSN " << insn.GetId() << " ";
  const IResource *res = insn.GetResource();
  os_ << res->GetClass()->GetName() << " " << res->GetId();
  // Details.
  os_ << " (" << insn.GetOperand() << ")";
  // Targets.
  os_ << " (";
  bool is_first = true;
  for (IState *st : insn.target_states_) {
    if (!is_first) {
      os_ << " ";
    }
    is_first = false;
    os_ << st->GetId();
  }
  os_ << ")";
  // Inputs.
  WriteInsnParams(insn.inputs_);
  // Outputs.
  WriteInsnParams(insn.outputs_);
  os_ << ")\n";
}

void ExpWriter::WriteInsnParams(const vector<IRegister *> &regs) {
  os_ << " (";
  bool is_first = true;
  for (auto &reg : regs) {
    if (!is_first) {
      os_ << " ";
    }
    os_ << reg->GetId();
    is_first = false;
  }
  os_ << ")";
}

void ExpWriter::WriteResourceTypes(const vector<IValueType> &types) {
  os_ << "(";
  bool is_first = true;
  for (auto &type : types) {
    if (!is_first) {
      os_ << " ";
    }
    WriteValueType(type);
    is_first = false;
  }
  os_ << ")";
}

void ExpWriter::WriteResourceParams(const ResourceParams &params,
				    const char *indent) {
  os_ << indent << "(PARAMS";
  vector<string> keys = params.GetParamKeys();
  if (!keys.empty()) {
    os_ << "\n";
  }
  bool is_first = true;
  for (string &key : keys) {
    if (!is_first) {
      os_ << "\n";
    }
    os_ << indent << " (" << key;
    vector<string> values = params.GetValues(key);
    for (string &value : values) {
      os_ << " " << value;
    }
    os_ << ")";
    is_first = false;
  }
  os_ << ")\n";
}

}  // namespace iroha
