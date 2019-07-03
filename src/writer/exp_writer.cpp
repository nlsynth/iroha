#include "writer/exp_writer.h"

#include "iroha/i_design.h"
#include "iroha/i_platform.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "numeric/numeric_type.h"

namespace iroha {
namespace writer {

ExpWriter::ExpWriter(const IDesign *design, ostream &os)
  : design_(design), os_(os) {
}

void ExpWriter::Write() {
  WriteResourceParams(*design_->GetParams(), "");
  for (auto *platform : design_->platforms_) {
    WritePlatform(*platform);
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
  if (resource::IsTaskCall(rc)) {
    WriteCalleeTaskDesc(res);
  }
  if (res.GetParentResource() != nullptr) {
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

void ExpWriter::WriteValue(const Numeric &value) {
  os_ << value.GetValue0();
}

void ExpWriter::WriteState(const IState &st) {
  os_ << "    (STATE " << st.GetId() << "\n";
  const IProfile &profile = st.GetProfile();
  if (profile.valid_ && (profile.raw_count_ > 0 || profile.normalized_count_ > 0)) {
    WriteProfile(profile);
  }
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
  // Depending insns.
  WriteDependingInsns(insn.depending_insns_);
  os_ << ")\n";
}

void ExpWriter::WriteProfile(const IProfile &profile) {
  os_ << "      (PROFILE " << profile.raw_count_ << " "
      << profile.normalized_count_ << ")\n";
}

void ExpWriter::WriteInsnParams(const vector<IRegister *> &regs) {
  vector<string> ids;
  for (auto &reg : regs) {
    ids.push_back(Util::Itoa(reg->GetId()));
  }
  os_ << " (" << Util::Join(ids, " ") << ")";
}

void ExpWriter::WriteDependingInsns(const vector<IInsn *> &insns) {
  vector<string> ids;
  for (auto &insn : insns) {
    ids.push_back(Util::Itoa(insn->GetId()));
  }
  os_ << " (" << Util::Join(ids, " ") << ")";
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

void ExpWriter::WritePlatform(const IPlatform &platform) {
  string name = platform.GetName();
  if (name.empty()) {
    name = "\"\"";
  }
  os_ << "(PLATFORM " << name;
  for (auto *def : platform.defs_) {
    WritePlatformDef(*def);
  }
  os_ << ")\n";
}

void ExpWriter::WritePlatformDef(const platform::Definition &def) {
  os_ << "\n  (DEF";
  if (def.condition_ != nullptr) {
    WritePlatformCondition(*(def.condition_));
  }
  if (def.value_ != nullptr) {
    WritePlatformValue(*(def.value_));
  }
  os_ << ")";
}

void ExpWriter::WritePlatformCondition(const platform::DefNode &cond) {
  os_ << "\n    (COND ";
  WriteNodeDef(cond);
  os_ << ")";
}

void ExpWriter::WritePlatformValue(const platform::DefNode &value) {
  os_ << "\n    (VALUE ";
  for (platform::DefNode *node : value.nodes_) {
    WriteNodeDef(*node);
  }
  os_ << ")";
}

void ExpWriter::WriteNodeDef(const platform::DefNode &node) {
  if (node.is_atom_) {
    if (node.str_.empty()) {
      os_ << node.num_;
    } else {
      os_ << node.str_;
    }
  } else {
    os_ << "(";
    bool is_first = true;
    for (platform::DefNode *child : node.nodes_) {
      if (!is_first) {
	os_ << " ";
      }
      is_first = false;
      WriteNodeDef(*child);
    }
    os_ << ")";
  }
}

}  // namespace writer
}  // namespace iroha
