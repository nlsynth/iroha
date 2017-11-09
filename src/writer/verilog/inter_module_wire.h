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
  void AddSharedWires(const vector<IResource *> &accessors,
		      const string &name, int width,
		      bool from_parent, bool drive_by_reg);

private:
  bool HasWire(Module *mod, const string &name);
  void AddWire(Module *mod, const string &name);
  void AddPort(Module *m, const string &name, int width, bool upward);

  Resource &res_;
  map<Module *, set<string> > has_wire_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_inter_module_wire_h_
