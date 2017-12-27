// -*- C++ -*-
#ifndef _writer_verilog_inter_module_wire_h_
#define _writer_verilog_inter_module_wire_h_

#include "writer/verilog/common.h"

#include <map>
#include <set>

namespace iroha {
namespace writer {
namespace verilog {

class InterModuleWire {
public:
  InterModuleWire(Resource &res);

  void AddWire(IResource &accessor, const string &name,
	       int width, bool from_parent, bool drive_by_reg);
  // Connects one shared value to many accessors in the module hierarchy.
  void AddSharedWires(const vector<IResource *> &accessors,
		      const string &name, int width,
		      bool from_parent, bool drive_by_reg);

private:
  bool HasWire(const Module *mod, const string &name);
  void AddWireName(const Module *mod, const string &name);
  void AddPort(Module *m, const string &name, int width, bool upward);

  Resource &res_;
  map<const Module *, set<string> > has_wire_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_inter_module_wire_h_
