// -*- C++ -*-
#ifndef _writer_verilog_inter_module_wire_h_
#define _writer_verilog_inter_module_wire_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

class InterModuleWire {
public:
  InterModuleWire(Resource &res);

  void AddWire(IResource &accessor, const string &name,
	       int width, bool from_parent, bool drive_by_reg);

private:
  Resource &res_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_inter_module_wire_h_
