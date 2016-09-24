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

  void AddCalleeTable(int mod_id, int table_id, IResource *res);
  void AddForeignReg(int mod_id, int table_id, int reg_id,
		     IResource *res);
  void AddParentModule(int parent_mod_id, IModule *mod);
  void AddChannelReaderWriter(IChannel *ch, bool is_r, int mod_id,
			      int tab_id, int res_id);
  void AddPortInput(int module_id, int table_id, int res_id, IResource *res);
  void AddArrayImage(IArray *array, int imageid);

  bool Resolve();

private:
  IRegister *FindForeignRegister(IModule *mod,
				 int table_id, int register_id);
  IResource *FindResource(IModule *mod,
			  int table_id, int resource_id);

  IDesign *design_;
  ExpBuilder *builder_;
  map<IResource *, int> callee_module_ids_;
  map<IResource *, int> table_ids_;
  map<IModule *, int> parent_module_ids_;
  struct ForeignRegister {
    int mod_id;
    int table_id;
    int reg_id;
  };
  map<IResource *, ForeignRegister> foreign_registers_;
  struct ChannelEndPoint {
    IChannel *ch;
    bool is_r;
    int mod_id;
    int tab_id;
    int res_id;
  };
  vector<ChannelEndPoint> channel_end_points_;
  struct PortInput {
    IResource *reader;
    int mod_id;
    int tab_id;
    int res_id;
  };
  vector<PortInput> port_inputs_;
  struct ArrayImage {
    IArray *array;
    int imageid;
  };
  vector<ArrayImage> array_images_;
};

}  // namespace builder
}  // namespace iroha

#endif  // _builder_tree_builder_h_
