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
  // module==nullptr: Can get register initializer.
  // module!=nullptr: Child module wirings.
  static void AddPort(const PortConfig &cfg, const string &name, int width,
		      bool dir_s2m,
		      bool is_master,
		      Module *module, Ports *ports, string *s);
  static void GenReadChannel(const PortConfig &cfg,
			     bool is_master, Module *module, Ports *ports,
			     string *s);
  static void GenWriteChannel(const PortConfig &cfg,
			      bool is_master, Module *module, Ports *ports,
			      string *s);
  void AddSramPorts();

  const IResource &res_;
  bool reset_polarity_;

  unique_ptr<Ports> ports_;
  PortConfig cfg_;
  int sram_addr_width_;
};

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_axi_axi_controller_h_
