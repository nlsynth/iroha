#include "design/design_tool.h"

#include <set>

namespace iroha {

DesignTool::DesignTool(IDesign *design) {
  if (design == nullptr) {
    design_ = new IDesign;
  } else {
    design_ = design;
  }
}

IDesign *DesignTool::GetDesign() {
  return design_;
}

void DesignTool::ValidateStateId(ITable *table) {
  if (table == nullptr) {
    for (auto *mod : design_->modules_) {
      for (auto *tab : mod->tables_) {
	ValidateStateId(tab);
      }
    }
    return;
  }
  set<int> used_ids;
  for (auto *st : table->states_) {
    used_ids.insert(st->GetId());
  }
  int last_id = 1;
  for (auto *st : table->states_) {
    if (st->GetId() < 0) {
      // Use first unused id.
      while (true) {
	if (used_ids.find(last_id) == used_ids.end()) {
	  break;
	}
	++last_id;
      }
      st->SetId(last_id);
      ++last_id;
    }
  }
}

}  // namespace iroha
