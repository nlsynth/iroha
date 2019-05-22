// -*- C++ -*-
#ifndef _numeric_numeric_manager_h_
#define _numeric_numeric_manager_h_

#include <set>

namespace iroha {

class ExtraWideValue;
class Numeric;

class NumericManager {
public:
  ~NumericManager();

  void MayPopulateStorage(Numeric *n);
  void StartGC();
  void MarkStorage(const Numeric *n);
  void DoGC();

private:
  std::set<ExtraWideValue *> values_;
  std::set<const ExtraWideValue *> marked_values_;
};

}  // iroha

#endif  // _numeric_numeric_manager_h_
