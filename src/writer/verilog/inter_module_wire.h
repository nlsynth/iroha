// -*- C++ -*-
#ifndef _writer_verilog_inter_module_wire_h_
#define _writer_verilog_inter_module_wire_h_

#include "writer/verilog/common.h"

#include <map>
#include <set>

namespace iroha {
namespace writer {
namespace verilog {

// InterModuleWire w(*this_resource);
// w.AddWire(....);
//
class InterModuleWire {
public:
  InterModuleWire(Resource &res);

  void AddWire(IResource &accessor, const string &name,
	       int width, bool from_parent, bool drive_by_reg);
  // Connects one shared value to many accessors in the module hierarchy.
  void AddSharedWires(const vector<const IResource *> &accessors,
		      const string &name, int width,
		      bool from_parent, bool drive_by_reg);

private:
  bool HasWire(const Module *mod, const string &name);
  bool HasPort(const Module *mod, const string &name);
  void AddWireName(const Module *mod, const string &name);
  void AddPort(Module *m, const string &name, int width, bool upward);
  void AddWireConnection(const IResource &accessor, const string &name,
			 int width, bool from_parent, bool drive_by_reg);
  // Adds a wire (not a reg) to the upper edge of the route.
  // module M1( /* w doesn't go up */ )
  //   wire w; // generates this!
  //   M2 m2_inst(.w(w) /* wire w from below */)
  //   M3 m3_inst(/* no wire w here */)
  void AddWireEdge(const IResource &accessor, const string &name,
		   int width, bool from_parent, bool drive_by_reg);

  Resource &res_;
  map<const Module *, set<string> > has_wire_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_inter_module_wire_h_
