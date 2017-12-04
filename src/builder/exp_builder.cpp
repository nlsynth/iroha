#include "builder/exp_builder.h"

#include "builder/reader.h"
#include "builder/fsm_builder.h"
#include "builder/tree_builder.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/module_import.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"

namespace iroha {
namespace builder {

IDesign *ExpBuilder::ReadDesign(const string &fn) {
  File *f = Reader::ReadFile(fn);
  if (!f) {
    return nullptr;
  }
  std::unique_ptr<File> deleter(f);
  if (f->exps.size()) {
    ExpBuilder builder;
    return builder.Build(f->exps);
  }
  return nullptr;
}

ExpBuilder::ExpBuilder() : has_error_(false) {
}

ExpBuilder::~ExpBuilder() {
}

IDesign *ExpBuilder::Build(vector<Exp *> &exps) {
  IDesign *design = new IDesign;
  tree_builder_.reset(new TreeBuilder(design, this));
  for (Exp *root : exps) {
    if (root->Size() == 0) {
      SetError() << "Empty toplevel expression\n";
      continue;
    }
    const string &element_name = root->GetHead();
    if (element_name == "MODULE") {
      IModule *module = BuildModule(root, design);
      if (module) {
	design->modules_.push_back(module);
      } else {
	SetError();
      }
    } else if (element_name == "PARAMS") {
      BuildResourceParams(root, design->GetParams());
    } else if (element_name == "CHANNEL") {
      BuildChannel(root, design);
    } else if (element_name == "ARRAY-IMAGE") {
      BuildArrayImage(root, design);
    } else {
      SetError() << "Unsupported toplevel expression";
    }
  }
  if (!HasError()) {
    tree_builder_->Resolve();
  }
  if (HasError()) {
    LOG(ERROR) << "Build failure: " << errors_.str();
    delete design;
    return nullptr;
  }
  return design;
}

IModule *ExpBuilder::BuildModule(Exp *e, IDesign *design) {
  if (e->Size() < 2) {
    SetError() << "Module requires its name";
    return nullptr;
  }
  int start = 2;
  string mod_name;
  int id = -1;
  // (MODULE name (...)) or (MODULE id name (...))
  if (Util::IsInteger(e->Str(1))) {
    id = Util::Atoi(e->Str(1));
    mod_name = e->Str(2);
    start = 3;
  } else {
    mod_name = e->Str(1);
  }
  IModule *module = new IModule(design, mod_name);
  module->SetId(id);
  for (int i = start; i < e->Size(); ++i) {
    Exp *element = e->vec[i];
    if (element->Size() == 0) {
      SetError();
      continue;
    }
    const string &element_name = element->GetHead();
    if (element_name == "TABLE") {
      ITable *table = BuildTable(element, module);
      if (table) {
	module->tables_.push_back(table);
      } else {
	SetError() << "Failed to build a table";
      }
    } else if (element_name == "PARAMS") {
      BuildResourceParams(element, module->GetParams());
    } else if (element_name == "PARENT") {
      if (element->Size() != 2) {
	SetError();
      } else {
	tree_builder_->AddParentModule(Util::Atoi(element->Str(1)), module);
      }
    } else if (element_name == "MODULE-IMPORT") {
      BuildModuleImport(element, module);
    } else {
      SetError() << "Unknown element in MODULE: " << element_name;
    }
  }
  return module;
}

ITable *ExpBuilder::BuildTable(Exp *e, IModule *module) {
  if (e->Size() < 2) {
    SetError() << "Insufficient elements for a table";
    return nullptr;
  }
  int id = -1;
  if (e->vec[1]->atom.str.empty()) {
    SetError() << "Expecting table id";
    return nullptr;
  } else {
    id = Util::Atoi(e->Str(1));
  }
  ITable *table = new ITable(module);
  table->SetId(id);
  int s = 2;
  // with/without table name.
  if (!e->Str(2).empty() || e->vec[2]->Size() == 0) {
    table->SetName(e->Str(2));
    s = 3;
  }
  for (int i = s; i < e->Size(); ++i) {
    Exp *element = e->vec[i];
    if (element->Size() == 0) {
      SetError() << "Expecting lists in table definition";
      return nullptr;
    }
    const string &tag = element->GetHead();
    if (tag == "REGISTERS") {
      BuildRegisters(element, table);
    } else if (tag == "RESOURCES") {
      BuildResources(element, table);
    } else if (tag == "INITIAL") {
      // Do the parsing later.
    } else if (tag == "STATE") {
      // Do the parsing later.
    } else {
      SetError() << "Unknown element in a table :" << tag;
    }
  }
  FsmBuilder fsm_builder(table, this);
  for (int i = 2; i < e->Size(); ++i) {
    Exp *element = e->vec[i];
    const string &tag = element->GetHead();
    if (tag == "STATE") {
      fsm_builder.AddState(element);
    } else if (tag == "INITIAL") {
      fsm_builder.SetInitialState(element);
    }
  }
  fsm_builder.ResolveInsns();
  return table;
}

void ExpBuilder::BuildRegisters(Exp *e, ITable *table) {
  for (int i = 1; i < e->Size(); ++i) {
    Exp *element = e->vec[i];
    if (element->Size() == 0) {
      SetError() << "Register should be defined as a list";
      return;
    }
    IRegister *reg = BuildRegister(element, table);
    if (reg != nullptr) {
      table->registers_.push_back(reg);
    }
  }
}

IRegister *ExpBuilder::BuildRegister(Exp *e, ITable *table) {
  if (e->GetHead() != "REGISTER") {
    SetError() << "Only REGISTER can be allowed";
    return nullptr;
  }
  if (e->Size() != 6) {
    SetError() << "Insufficient parameters for a register";
    return nullptr;
  }
  const string &name = e->Str(2);
  int id = Util::Atoi(e->Str(1));
  IRegister *reg = new IRegister(table, name);
  reg->SetId(id);
  const string &type = e->Str(3);
  if (type == "REG") {
    // do nothing.
  } else if (type == "CONST") {
    reg->SetConst(true);
  } else if (type == "WIRE") {
    reg->SetStateLocal(true);
  } else {
    SetError() << "Unknown register class: " << type;
  }
  BuildValueType(e->vec[4], &reg->value_type_);
  if (!e->Str(5).empty()) {
    const string &ini = e->Str(5);
    Numeric value;
    value.SetValue0(Util::AtoULL(ini));
    value.type_ = reg->value_type_;
    reg->SetInitialValue(value);
  }
  return reg;
}

void ExpBuilder::BuildResources(Exp *e, ITable *table) {
  for (int i = 1; i < e->Size(); ++i) {
    Exp *element = e->vec[i];
    if (element->Size() == 0) {
      SetError() << "Resource should be defined as a list";
      return;
    }
    IResource *res = BuildResource(element, table);
    if (res != nullptr &&
	!resource::IsTransition(*res->GetClass())) {
      table->resources_.push_back(res);
    }
  }
}

IResource *ExpBuilder::BuildResource(Exp *e, ITable *table) {
  if (e->GetHead() != "RESOURCE") {
    SetError() << "Only RESOURCE can be allowed";
    return nullptr;
  }
  if (e->Size() < 6) {
    SetError() << "Malformed RESOURCE";
    return nullptr;
  }
  const string &klass = e->Str(2);
  IDesign *design = table->GetModule()->GetDesign();
  IResourceClass *rc = nullptr;
  for (auto *c : design->resource_classes_) {
    if (c->GetName() == klass) {
      rc = c;
    }
  }
  if (rc == nullptr) {
    if (klass == "ext_input" || klass == "ext_output") {
      SetError() << "Please fix ext_{in, out}put to ext-{in, out}put";
    }
    SetError() << "Unknown resource class: " << klass;
    return nullptr;
  }
  IResource *res = nullptr;
  if (klass == resource::kTransition) {
    // Use pre installed resource.
    res = table->resources_[0];
  } else {
    res = new IResource(table, rc);
  }
  int id = Util::Atoi(e->Str(1));
  res->SetId(id);
  BuildParamTypes(e->vec[3], &res->input_types_);
  BuildParamTypes(e->vec[4], &res->output_types_);
  BuildResourceParams(e->vec[5], res->GetParams());
  for (int i = 6; i < e->Size(); ++i) {
    Exp *element = e->vec[i];
    if (element->Size() == 0) {
      SetError() << "Empty additional resource parameter";
      return nullptr;
    }
    const string &element_name = element->GetHead();
    if (element_name == "ARRAY") {
      BuildArray(element, res);
    } else if (element_name == "CALLEE-TABLE") {
      if (element->Size() == 3) {
	tree_builder_->AddCalleeTable(Util::Atoi(element->Str(1)),
				      Util::Atoi(element->Str(2)),
				      res);
      } else {
	SetError() << "Invalid module spec";
	return nullptr;
      }
    } else if (element_name == "FOREIGN-REG") {
      int sz = element->Size();
      if (sz == 4) {
	tree_builder_->AddForeignReg(Util::Atoi(element->Str(1)),
				     Util::Atoi(element->Str(2)),
				     Util::Atoi(element->Str(3)),
				     res);
      } else {
	SetError() << "Invalid foreign reg spec";
	return nullptr;
      }
    } else if (element_name == "PARENT-RESOURCE" ||
	       // For compatibility
	       element_name == "SHARED-REG") {
      int sz = element->Size();
      if (sz == 4) {
	tree_builder_->AddParentResource(Util::Atoi(element->Str(1)),
					 Util::Atoi(element->Str(2)),
					 Util::Atoi(element->Str(3)),
					 res);
      } else {
	SetError() << "Invalid PARENT-RESOURCE spec";
      }
    } else {
      SetError() << "Invalid additional resource parameter";
      return nullptr;
    }
  }
  return res;
}

void ExpBuilder::BuildArray(Exp *e, IResource *res) {
  int sz = e->Size();
  if (sz != 5 && sz != 6) {
    SetError() << "Malformed array description";
    return;
  }
  int address_width = Util::Atoi(e->Str(1));
  IValueType data_type;
  BuildValueType(e->vec[2], &data_type);
  bool is_external;
  const string &v = e->Str(3);
  if (v == "EXTERNAL") {
    is_external = true;
  } else if (v == "INTERNAL") {
    is_external = false;
  } else {
    SetError() << "Array should be either INTERNAL or EXTERNAL";
    return;
  }
  bool is_ram;
  const string &w = e->Str(4);
  if (w == "RAM") {
    is_ram = true;
  } else if (w == "ROM") {
    is_ram = false;
  } else {
    SetError() << "Array should be either RAM or ROM";
    return;
  }
  IArray *array = new IArray(res, address_width, data_type,
			     is_external, is_ram);
  res->SetArray(array);
  if (sz == 6) {
    int imid = Util::Atoi(e->Str(5));
    tree_builder_->AddArrayImage(array, imid);
  }
}

void ExpBuilder::BuildResourceParams(Exp *e, ResourceParams *params) {
  for (int i = 1; i < e->Size(); ++i) {
    Exp *t = e->vec[i];
    vector<string> s;
    for (int j = 1; j < t->Size(); ++j) {
      s.push_back(t->Str(j));
    }
    params->SetValues(t->GetHead(), s);
  }
}

void ExpBuilder::BuildParamTypes(Exp *e, vector<IValueType> *types) {
  for (int i = 0; i < e->Size(); ++i) {
    IValueType type;
    BuildValueType(e->vec[i], &type);
    types->push_back(type);
  }
}

void ExpBuilder::BuildValueType(Exp *e, IValueType *vt) {
  if (e->Size() != 2) {
    SetError() << "value type shoule be like (UINT 32)";
    return;
  }
  Exp *t = e->vec[0];
  Exp *w = e->vec[1];
  const string &width = w->atom.str;
  vt->SetWidth(Util::Atoi(width));
  const string &s = t->atom.str;
  if (s == "INT") {
    vt->SetIsSigned(true);
  } else if (s != "UINT") {
    SetError() << "Type should be INT or UINT";
  }
}

ostream &ExpBuilder::SetError() {
  has_error_ = true;
  const string &s = errors_.str();
  if (!s.empty() && s.c_str()[s.size() - 1] != '\n') {
    errors_ << "\n";
  }
  return errors_;
}

void ExpBuilder::BuildChannel(Exp *e, IDesign *design) {
  IChannel *ch = new IChannel(design);
  design->channels_.push_back(ch);
  ch->SetId(Util::Atoi(e->Str(1)));
  int idx = 2;
  if (e->vec[idx]->Size() == 0) {
    ch->SetName(e->Str(idx));
    ++idx;
  }
  IValueType vt;
  BuildValueType(e->vec[idx], &vt);
  ++idx;
  ch->SetValueType(vt);
  BuildChannelReaderWriter(e->vec[idx], true, ch);
  BuildChannelReaderWriter(e->vec[idx + 1], false, ch);
  idx += 2;
  if (e->Size() > idx) {
    if (e->vec[idx]->GetHead() == "PARAMS") {
      BuildResourceParams(e->vec[idx], ch->GetParams());
    }
  }
}

void ExpBuilder::BuildArrayImage(Exp *e, IDesign *design) {
  if (e->Size() != 4) {
    SetError() << "Invalid array image";
    return;
  }
  IArrayImage *array_image = new IArrayImage(design);
  array_image->SetId(Util::Atoi(e->Str(1)));
  array_image->SetName(e->Str(2));
  Exp *array = e->vec[3];
  for (int i = 0; i < array->Size(); ++i) {
    array_image->values_.push_back(Util::AtoULL(array->Str(i)));
  }
  design->array_images_.push_back(array_image);
}

void ExpBuilder::BuildChannelReaderWriter(Exp *e, bool is_r, IChannel *ch) {
  if (e->Size() == 0) {
    return;
  }
  tree_builder_->AddChannelReaderWriter(ch, is_r,
					Util::Atoi(e->Str(0)),
					Util::Atoi(e->Str(1)),
					Util::Atoi(e->Str(2)));
}

void ExpBuilder::BuildModuleImport(Exp *e, IModule *mod) {
  if (e->Size() < 2) {
    SetError() << "Malformed module import";
    return;
  }
  ModuleImport *mi = new ModuleImport(mod, e->Str(1));
  for (int i = 2; i < e->Size(); ++i) {
    Exp *t = e->vec[i];
    if (t->Str(0) == "TAP") {
      BuildModuleImportTap(t, mi);
    } else {
      SetError() << "Unknown item in module-import:" << t->Str(0);
    }
  }
  mod->SetModuleImport(mi);
}

void ExpBuilder::BuildModuleImportTap(Exp *e, ModuleImport *mi) {
  if (e->Size() < 4) {
    SetError() << "Malformed module import tap";
    return;
  }
  ModuleImportTap *tap = new ModuleImportTap();
  tap->source = e->Str(1);
  tap->tag = e->Str(2);
  BuildModuleImportTapDesc(e->vec[3], tap);
  mi->taps_.push_back(tap);
}

void ExpBuilder::BuildModuleImportTapDesc(Exp *e, ModuleImportTap *tap) {
  tap->resource_class = e->Str(0);
  if (e->Size() == 4) {
    tree_builder_->AddModuleImportTap(Util::Atoi(e->Str(1)),
				      Util::Atoi(e->Str(2)),
				      Util::Atoi(e->Str(3)), tap);
  }
}

bool ExpBuilder::HasError() {
  return has_error_;
}

}  // namespace builder
}  // namespace iroha
