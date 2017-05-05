// -*- C++ -*-
#ifndef _writer_verilog_axi_master_controller_h_
#define _writer_verilog_axi_master_controller_h_

#include "writer/verilog/axi/axi_controller.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

class MasterController : public AxiController {
public:
  MasterController(const IResource &res,
		   bool reset_polarity);
  ~MasterController();

  void Write(ostream &os);

  static void AddPorts(Module *mod, bool r, bool w,
		       string *s);

private:
  void OutputMainFsm(ostream &os);
  void ReadState(ostream &os);
  void OutputWriterFsm(ostream &os);

  bool r_, w_;
  int burst_len_;
};

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif // _writer_verilog_axi_master_controller_h_
