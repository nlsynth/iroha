// -*- C++ -*-
//
// Kills empty tables.
//
#ifndef _opt_clean_empty_table_h_
#define _opt_clean_empty_table_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace clean {

class CleanEmptyTablePass : public Pass {
 public:
  virtual ~CleanEmptyTablePass();
  static Pass *Create();

 private:
  virtual bool ApplyForDesign(IDesign *design);
  virtual bool ApplyForModule(const string &key, IModule *module);
  virtual bool ApplyForTable(const string &key, ITable *table);

  bool IsEmpty(ITable *tab);
};

}  // namespace clean
}  // namespace opt
}  // namespace iroha

#endif  // _opt_clean_empty_table_h_
