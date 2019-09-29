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
  const WireSet *GetWireSet() const;
  MuxNode *GetRootNode() const;

private:
  const WireSet *ws_;
  unique_ptr<Ports> ports_;
  MuxNode *root_node_;
  int num_nodes_;

  MuxNode *BuildNodes(const vector<AccessorInfo *> &acs);
  void BalanceNode(MuxNode *node);
  void DeleteNode(MuxNode *node);
  int MaxFanOut();
};

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_wire_mux_h_
