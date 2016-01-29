// -*- C++ -*-
#ifndef _builder_tree_builder_h_
#define _builder_tree_builder_h_

#include "iroha/common.h"

#include <map>

namespace iroha {
namespace builder {

class Exp;
class ExpBuilder;

class TreeBuilder {
public:
  TreeBuilder(IDesign *design, ExpBuilder *builder);

  void AddCalleeTable(const string &mod_name, int table_id, IResource *res);
  void AddParentModule(const string &name, IModule *mod);

  bool Resolve();

private:
  IDesign *design_;
  ExpBuilder *builder_;
  map<IResource *, string> callee_module_names_;
  map<IResource *, int> table_ids_;
  map<IModule *, string> parent_module_names_;
};

}  // namespace builder
}  // namespace iroha

#endif  // _builder_tree_builder_h_
