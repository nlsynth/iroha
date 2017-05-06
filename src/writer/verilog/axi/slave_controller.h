// -*- C++ -*-
#ifndef _writer_verilog_axi_slave_controller_h_
#define _writer_verilog_axi_slave_controller_h_

#include "writer/verilog/axi/axi_controller.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

class SlaveController : public AxiController {
public:
  SlaveController(const IResource &res,
		   bool reset_polarity);
  ~SlaveController();

  void Write(ostream &os);

  static void AddPorts(Module *mod, string *s);

protected:
  void OutputFSM(ostream &os);
};

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif // _writer_verilog_axi_slave_controller_h_
