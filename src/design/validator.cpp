#include "design/validator.h"

#include "iroha/i_design.h"

#include <set>

namespace iroha {

template<class T>
static void ValidateId(vector<T *> &v, set<int> &used_ids) {
  int last_id = 1;
  for (T *t : v) {
    if (t->GetId() < 0) {
      // Use first unused id.
      while (true) {
	if (used_ids.find(last_id) == used_ids.end()) {
	  break;
	}
	++last_id;
      }
      t->SetId(last_id);
      ++last_id;
    }
  }
}

template<class T>
static void ValidateVectorId(vector<T *> &v) {
  set<int> used_ids;
  for (auto *e : v) {
    used_ids.insert(e->GetId());
  }
  ValidateId(v, used_ids);
}

void Validator::Validate(IDesign *design) {
  ValidateModuleId(design);
  ValidateChannelId(design);
  ValidateArrayImageId(design);
  for (auto *mod : design->modules_) {
    ValidateTableId(mod);
    for (auto *tab : mod->tables_) {
      ValidateTable(tab);
    }
    // Validate reg names after ids.
    ValidateRegName(mod);
  }
}

void Validator::ValidateTable(ITable *table) {
  ValidateStateId(table);
  ValidateInsnId(table);
  ValidateRegisterId(table);
  ValidateResourceId(table);
}

void Validator::ValidateModuleId(IDesign *design) {
  ValidateVectorId(design->modules_);
}

void Validator::ValidateChannelId(IDesign *design) {
  ValidateVectorId(design->channels_);
}

void Validator::ValidateArrayImageId(IDesign *design) {
  ValidateVectorId(design->array_images_);
}

void Validator::ValidateStateId(ITable *table) {
  ValidateVectorId(table->states_);
}

void Validator::ValidateTableId(IModule *mod) {
  ValidateVectorId(mod->tables_);
}

void Validator::ValidateInsnId(ITable *table) {
  set<int> used_ids;
  vector<IInsn *> insns;
  for (auto *st : table->states_) {
    for (auto *insn : st->insns_) {
      used_ids.insert(insn->GetId());
      insns.push_back(insn);
    }
  }
  ValidateId(insns, used_ids);
}

void Validator::ValidateResourceId(ITable *table) {
  ValidateVectorId(table->resources_);
}

void Validator::ValidateRegisterId(ITable *table) {
  ValidateVectorId(table->registers_);
}

void Validator::ValidateRegName(IModule *mod) {
  set<string> names;
  for (auto *tab : mod->tables_) {
    for (auto *reg : tab->registers_) {
      string name = reg->GetName();
      if (name.empty() || names.find(name) != names.end()) {
	name = GetUniqueName(names, reg);
	reg->SetName(name);
      }
      names.insert(name);
    }
  }
}

string Validator::GetUniqueName(set<string> &names, IRegister *reg) {
  string name = reg->GetName();
  if (name.empty()) {
    name = "anon";
  }
  string r = name + "_" + Util::Itoa(reg->GetId());
  if (names.find(r) == names.end()) {
    return r;
  }
  string s;
  int n = 0;
  do {
    ++n;
    s = r + "_" + Util::Itoa(n);
  } while (names.find(s) != names.end());
  return s;
}

}  // namespace iroha
