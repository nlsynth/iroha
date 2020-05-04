// -*- C++ -*-
#ifndef _writer_verilog_axi_axi_controller_h_
#define _writer_verilog_axi_axi_controller_h_

#include "writer/verilog/common.h"
#include "writer/verilog/axi/axi_port.h"  // for PortConfig

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

class AxiController {
public:
  AxiController(const IResource &res,
		bool reset_polarity);
  ~AxiController();

  static string ResetName(bool polarity);

protected:
  static void WriteModuleHeader(const string &name, ostream &os);
  static void WriteModuleFooter(const string &name, ostream &os);
  void AddSramPorts();

  const IResource &res_;
  bool reset_polarity_;

  unique_ptr<PortSet> ports_;
  PortConfig cfg_;
};

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_axi_axi_controller_h_
