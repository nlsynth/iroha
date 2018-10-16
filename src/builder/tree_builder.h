// -*- C++ -*-
#ifndef _builder_tree_builder_h_
#define _builder_tree_builder_h_

#include "iroha/common.h"

#include <map>

namespace iroha {
namespace builder {

class Exp;
class DesignBuilder;

class TreeBuilder {
public:
  TreeBuilder(IDesign *design, DesignBuilder *builder);

  void AddCalleeTable(int mod_id, int table_id, IResource *res);
  void AddParentModule(int parent_mod_id, IModule *mod);
  void AddParentResource(int module_id, int table_id, int res_id,
			 IResource *res);
  void AddArrayImage(IArray *array, int imageid);
  void AddModuleImportTap(int module_id, int table_id, int res_id,
			  ModuleImportTap *tap);

  bool Resolve();

private:
  IResource *FindResource(IModule *mod,
			  int table_id, int resource_id);

  IDesign *design_;
  DesignBuilder *builder_;
  map<IResource *, int> callee_module_ids_;
  map<IResource *, int> table_ids_;
  map<IModule *, int> parent_module_ids_;
  struct ParentResource {
    IResource *reader;
    int mod_id;
    int tab_id;
    int res_id;
  };
  vector<ParentResource> parent_resources_;
  struct ArrayImage {
    IArray *array;
    int imageid;
  };
  vector<ArrayImage> array_images_;
  struct Tap {
    ModuleImportTap *tap;
    int mod_id;
    int tab_id;
    int res_id;
  };
  vector<Tap> taps_;
};

}  // namespace builder
}  // namespace iroha

#endif  // _builder_tree_builder_h_
