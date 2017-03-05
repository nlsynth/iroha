#include "writer/exp_writer.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/module_import.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"

namespace iroha {

ExpWriter::ExpWriter(const IDesign *design, ostream &os)
  : design_(design), os_(os) {
}

void ExpWriter::Write() {
  WriteResourceParams(*design_->GetParams(), "");
  for (auto *ch : design_->channels_) {
    WriteChannel(*ch);
  }
  for (auto *im : design_->array_images_) {
    WriteArrayImage(*im);
  }
  for (auto *mod : design_->modules_) {
    WriteModule(*mod);
  }
}

void ExpWriter::WriteModule(const IModule &mod) {
  os_ << "(MODULE " << mod.GetId() << " " << mod.GetName() << "\n";
  IModule *parent = mod.GetParentModule();
  if (parent != nullptr) {
    os_ << "  (PARENT " << parent->GetId() << ")\n";
  }
  ModuleImport *mi = mod.GetModuleImport();
  if (mi != nullptr) {
    os_ << "  (MODULE-IMPORT " << mi->GetFileName() << "\n";
    for (auto *t : mi->taps_) {
      os_ << "   (TAP " << t->source << " ";
      if (t->tag.empty()) {
	os_ << "()";
      } else {
	os_ << t->tag;
      }
      os_ << " (" << t->resource_class;
      if (t->resource != nullptr) {
	ITable *tab = t->resource->GetTable();
	IModule *mod = tab->GetModule();
	os_ << " " << mod->GetId() << " " << tab->GetId() << " "
	    << t->resource->GetId();
      }
      os_ << ")";
      os_ << ")\n";
    }
    os_ << "  )\n";
  }
  WriteResourceParams(*mod.GetParams(), "  ");
  for (auto *tab : mod.tables_) {
    WriteTable(*tab);
  }
  os_ << ")\n";
}

void ExpWriter::WriteTable(const ITable &tab) {
  os_ << "  (TABLE " << tab.GetId();
  string name = tab.GetName();
  if (name.empty()) {
    os_ << " ()";
  } else {
    os_ << " " << name;
  }
  os_ << "\n";
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
  WriteResourceParams(*params, "        ");
  if (resource::IsArray(rc) || res.GetArray() != nullptr) {
    WriteArrayDesc(res);
  }
  if (resource::IsForeignRegister(rc)) {
    WriteForeignRegDesc(res);
  }
  if (resource::IsTaskCall(rc)) {
    WriteCalleeTaskDesc(res);
  }
  if (resource::IsSharedRegReader(rc) ||
      resource::IsSharedRegWriter(rc) ||
      resource::IsSharedMemoryReader(rc) ||
      resource::IsSharedMemoryWriter(rc) ||
      resource::IsAxiPort(rc)) {
    WriteParentResourceDesc(res);
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
  IArrayImage *im = array->GetArrayImage();
  if (im != nullptr) {
    os_ << " " << im->GetId();
  }
  os_ << ")\n";
}

void ExpWriter::WriteForeignRegDesc(const IResource &res) {
  os_ << "        (FOREIGN-REG ";
  IRegister *foreign_reg = res.GetForeignRegister();
  IModule *foreign_mod = foreign_reg->GetTable()->GetModule();
  os_ << " " << foreign_mod->GetId();
  os_ << res.GetTable()->GetId();
  os_ << " ";
  os_ << foreign_reg->GetId();
  os_ << ")\n";
}

void ExpWriter::WriteCalleeTaskDesc(const IResource &res) {
  const ITable *table = res.GetCalleeTable();
  CHECK(table) << "callee table isn't specified";
  const IModule *mod = table->GetModule();
  os_ << "        (CALLEE-TABLE " << mod->GetId() << " "
      << table->GetId() << ")\n";
}

void ExpWriter::WriteParentResourceDesc(const IResource &res) {
  const IResource *source = res.GetParentResource();
  CHECK(source) << "Missing PARENT-RESOURCE";
  const ITable *table = source->GetTable();
  const IModule *mod = table->GetModule();
  os_ << "        (PARENT-RESOURCE " <<  mod->GetId() << " "
      << table->GetId() << " " << source->GetId() << ")\n";
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
  os_ << "(";
  if (type.IsSigned()) {
    os_ << "INT";
  } else {
    os_ << "UINT";
  }
  os_ << " " << type.GetWidth() << ")";
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
      os_ << " ";
      WriteStr(value);
    }
    os_ << ")";
    is_first = false;
  }
  os_ << ")\n";
}

void ExpWriter::WriteChannel(const IChannel &ch) {
  os_ << "(CHANNEL " << ch.GetId() << " ";
  if (ch.GetName().empty()) {
    os_ << "()";
  } else {
    os_ << ch.GetName();
  }
  os_ << " ";
  WriteValueType(ch.GetValueType());
  os_ << " ";
  IResource *reader = ch.GetReader();
  if (reader == nullptr) {
    os_ << "()";
  } else {
    WriteResourceDesc(*reader);
  }
  os_ << " ";
  IResource *writer = ch.GetWriter();
  if (writer == nullptr) {
    os_ << "()";
  } else {
    WriteResourceDesc(*writer);
  }
  os_ << "\n";
  WriteResourceParams(*(ch.GetParams()), "  ");
  os_ << ")\n";
}

void ExpWriter::WriteArrayImage(const IArrayImage &im) {
  os_ << "(ARRAY-IMAGE " << im.GetId() << " ";
  if (im.GetName().empty()) {
    os_ << "()";
  } else {
    os_ << im.GetName();
  }
  os_ << "\n"
      << " (";
  bool f = true;
  for (auto &v : im.values_) {
    if (!f) {
      os_ << " ";
    }
    os_ << v;
    f = false;
  }
  os_ << "))\n";
}

void ExpWriter::WriteResourceDesc(const IResource &res) {
  ITable *tab = res.GetTable();
  IModule *mod = tab->GetModule();
  os_ << "(" << mod->GetId() << " " << tab->GetId() << " "
      << res.GetId() << ")";
}

void ExpWriter::WriteStr(const string &str) {
  bool need_quote = false;
  if (str.find(" ") != string::npos) {
    need_quote = true;
  }
  if (need_quote) {
    os_ << "\"";
  }
  os_ << str;
  if (need_quote) {
    os_ << "\"";
  }
}

}  // namespace iroha
