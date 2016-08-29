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

void Validator::Validate(IDesign *design) {
  ValidateModuleId(design);
  ValidateChannelId(design);
  for (auto *mod : design->modules_) {
    ValidateTableId(mod);
    for (auto *tab : mod->tables_) {
      ValidateTable(tab);
    }
  }
}

void Validator::ValidateTable(ITable *table) {
  ValidateStateId(table);
  ValidateInsnId(table);
  ValidateRegisterId(table);
  ValidateResourceId(table);
}

void Validator::ValidateModuleId(IDesign *design) {
  set<int> used_ids;
  for (auto *mod : design->modules_) {
    used_ids.insert(mod->GetId());
  }
  ValidateId(design->modules_, used_ids);
}

void Validator::ValidateChannelId(IDesign *design) {
  set<int> used_ids;
  for (auto *ch : design->channels_) {
    used_ids.insert(ch->GetId());
  }
  ValidateId(design->channels_, used_ids);
}

void Validator::ValidateStateId(ITable *table) {
  set<int> used_ids;
  for (auto *st : table->states_) {
    used_ids.insert(st->GetId());
  }
  ValidateId(table->states_, used_ids);
}

void Validator::ValidateTableId(IModule *mod) {
  set<int> used_ids;
  for (auto *tab : mod->tables_) {
    used_ids.insert(tab->GetId());
  }
  ValidateId(mod->tables_, used_ids);
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
  set<int> used_ids;
  for (auto *res : table->resources_) {
    used_ids.insert(res->GetId());
  }
  ValidateId(table->resources_, used_ids);
}

void Validator::ValidateRegisterId(ITable *table) {
  set<int> used_ids;
  for (auto *reg : table->registers_) {
    used_ids.insert(reg->GetId());
  }
  ValidateId(table->registers_, used_ids);
}

}  // namespace iroha
