#include "numeric/numeric_manager.h"

// This class manages storage of extra wide values.

#include "numeric/numeric_type.h"

namespace iroha {

NumericManager::~NumericManager() {
}

void NumericManager::MayPopulateStorage(Numeric *n) {
  if (!n->type_.IsExtraWide()) {
    return;
  }
  ExtraWideValue *ev = new ExtraWideValue();
  n->GetMutableArray()->extra_wide_value_ = ev;
  values_.insert(ev);
}

void NumericManager::StartGC() {
  marked_values_.clear();
}

void NumericManager::MarkStorage(const Numeric *n) {
  // This may contain addresses of undefined locations.
  marked_values_.insert(n->GetArray().extra_wide_value_);
}

void NumericManager::DoGC() {
  std::set<ExtraWideValue *> live;
  for (auto *v : values_) {
    if (marked_values_.find(v) == marked_values_.end()) {
      delete v;
    } else {
      live.insert(v);
    }
  }
  values_ = live;
  marked_values_.clear();
}

}  // iroha
