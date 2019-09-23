// -*- C++ -*-
#ifndef _writer_verilog_wire_mux_h_
#define _writer_verilog_wire_mux_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

class Mux {
public:
  Mux(const WireSet *ws);
  ~Mux();

  static void Write(const WireSet *ws, ostream &os);

  void DoWrite(ostream &os);

private:
  const WireSet *ws_;
  unique_ptr<Ports> ports_;
  MuxNode *root_node_;

  MuxNode *BuildNodes(const vector<AccessorInfo *> &acs);
  void DeleteNode(MuxNode *node);
};

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_wire_mux_h_
