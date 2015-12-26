#include "builder/exp_builder.h"

#include "builder/reader.h"
#include "builder/fsm_builder.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"

namespace iroha {

IDesign *ExpBuilder::ReadDesign(const string &fn) {
  iroha::File *f = iroha::Reader::ReadFile(fn);
  if (f->exps.size()) {
    ExpBuilder builder;
    return builder.Build(f->exps);
  }
  return nullptr;
}

ExpBuilder::ExpBuilder() : has_error_(false) {
}

IDesign *ExpBuilder::Build(vector<Exp *> &exps) {
  IDesign *design = new IDesign;
  for (Exp *root : exps) {
    if (root->vec.size() == 0) {
      SetError() << "Empty toplevel expression\n";
      continue;
    }
    if (root->vec[0]->atom.str == "MODULE") {
      IModule *module = BuildModule(root, design);
      if (module) {
	design->modules_.push_back(module);
      } else {
	SetError();
      }
    } else {
      SetError() << "Unsupported toplevel expression";
    }
  }
  if (HasError()) {
    LOG(ERROR) << "Build failure: " << errors_.str();
  }
  return design;
}

IModule *ExpBuilder::BuildModule(Exp *e, IDesign *design) {
  if (e->vec.size() < 3) {
    SetError() << "Insufficient elements for a module";
    return nullptr;
  }
  IModule *module = new IModule(design, e->vec[1]->atom.str);
  for (int i = 2; i < e->vec.size(); ++i) {
    if (e->vec[i]->vec.size() == 0) {
      SetError();
    } else if (e->vec[i]->vec[0]->atom.str == "TABLE") {
      ITable *table = BuildTable(e->vec[i], module);
      if (table) {
	module->tables_.push_back(table);
      } else {
	SetError() << "Failed to build a table";
      }
    } else {
      SetError();
    }
  }
  return module;
}

ITable *ExpBuilder::BuildTable(Exp *e, IModule *module) {
  if (e->vec.size() < 2) {
    SetError() << "Insufficient elements for a table";
    return nullptr;
  }
  int id = -1;
  if (e->vec[1]->atom.str.empty()) {
    SetError() << "Expecting table id";
    return nullptr;
  } else {
    id = Util::Atoi(e->vec[1]->atom.str);
  }
  ITable *table = new ITable(module);
  table->SetId(id);
  for (int i = 2; i < e->vec.size(); ++i) {
    if (e->vec[i]->vec.size() == 0) {
      SetError() << "Expecting lists in table definition";
      return nullptr;
    }
    const string &tag = e->vec[i]->vec[0]->atom.str;
    if (tag == "REGISTERS") {
      BuildRegisters(e->vec[i], table);
    } else if (tag == "RESOURCES") {
      BuildResources(e->vec[i], table);
    } else if (tag == "INITIAL") {
      // Do the parsing later.
    } else if (tag == "STATE") {
      // Do the parsing later.
    } else {
      SetError() << "Unknown element in a table :" << tag;
    }
  }
  FsmBuilder fsm_builder(table, this);
  for (int i = 2; i < e->vec.size(); ++i) {
    const string &tag = e->vec[i]->vec[0]->atom.str;
    if (tag == "STATE") {
      fsm_builder.AddState(e->vec[i]);
    } else if (tag == "INITIAL") {
      fsm_builder.SetInitialState(e->vec[i]);
    }
  }
  fsm_builder.ResolveInsns();
  return table;
}

void ExpBuilder::BuildRegisters(Exp *e, ITable *table) {
  for (int i = 1; i < e->vec.size(); ++i) {
    if (e->vec[i]->vec.size() == 0) {
      SetError() << "Register should be defined as a list";
      return;
    }
    IRegister *reg = BuildRegister(e->vec[i], table);
    if (reg != nullptr) {
      table->registers_.push_back(reg);
    }
  }
}

IRegister *ExpBuilder::BuildRegister(Exp *e, ITable *table) {
  if (e->vec[0]->atom.str != "REGISTER") {
    SetError() << "Only REGISTER can be allowed";
    return nullptr;
  }
  if (e->vec.size() != 6) {
    SetError() << "Insufficient parameters for a register";
    return nullptr;
  }
  const string &name = e->vec[2]->atom.str;
  int id = Util::Atoi(e->vec[1]->atom.str);
  IRegister *reg = new IRegister(table, name);
  reg->SetId(id);
  const string &type = e->vec[3]->atom.str;
  if (type == "REG") {
    // do nothing.
  } else if (type == "CONST") {
    reg->SetConst(true);
  } else if (type == "WIRE") {
    reg->SetStateLocal(true);
  } else {
    SetError() << "Unknown register class: " << type;
  }
  const string &width = e->vec[4]->atom.str;
  reg->value_type_.SetWidth(Util::Atoi(width));
  if (!e->vec[5]->atom.str.empty()) {
    const string &ini = e->vec[5]->atom.str;
    IValue value;
    value.value_ = Util::Atoi(ini);
    value.type_ = reg->value_type_;
    reg->SetInitialValue(value);
  }
  return reg;
}

void ExpBuilder::BuildResources(Exp *e, ITable *table) {
  for (int i = 1; i < e->vec.size(); ++i) {
    if (e->vec[i]->vec.size() == 0) {
      SetError() << "Resource should be defined as a list";
      return;
    }
    IResource *res = BuildResource(e->vec[i], table);
    if (res != nullptr) {
      table->resources_.push_back(res);
    }
  }
}

IResource *ExpBuilder::BuildResource(Exp *e, ITable *table) {
  if (e->vec[0]->atom.str != "RESOURCE") {
    SetError() << "Only RESOURCE can be allowed";
    return nullptr;
  }
  if (e->vec.size() != 5) {
    SetError() << "Malformed RESOURCE";
    return nullptr;
  }
  const string &klass = e->vec[2]->atom.str;
  if (klass == resource::kTransition) {
    // Ignore pre-installed resource.
    return nullptr;
  }
  IDesign *design = table->GetModule()->GetDesign();
  IResourceClass *rc = nullptr;
  for (auto *c : design->resource_classes_) {
    if (c->GetName() == klass) {
      rc = c;
    }
  }
  if (rc == nullptr) {
    SetError() << "Unknown resource class: " << klass;
    return nullptr;
  }
  IResource *res = new IResource(table, rc);
  int id = Util::Atoi(e->vec[1]->atom.str);
  res->SetId(id);
  BuildParamTypes(e->vec[3], &res->input_types_);
  BuildParamTypes(e->vec[4], &res->output_types_);
  return res;
}

void ExpBuilder::BuildParamTypes(Exp *e, vector<IValueType> *types) {
  for (Exp *t : e->vec) {
    IValueType type;
    type.SetWidth(Util::Atoi(t->atom.str));
    types->push_back(type);
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

bool ExpBuilder::HasError() {
  return has_error_;
}

}  // namespace iroha
