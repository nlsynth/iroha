// -*- C++ -*-
#ifndef _writer_verilog_axi_axi_controller_h_
#define _writer_verilog_axi_axi_controller_h_

#include "writer/verilog/common.h"

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
  static void AddPort(const string &name, int width, bool dir_s2m,
		      bool is_master,
		      Module *module, Ports *ports, string *s);
  static void GenReadChannel(bool is_master, Module *module, Ports *ports,
			     string *s);
  static void GenWriteChannel(bool is_master, Module *module, Ports *ports,
			      string *s);
  void AddSramPorts();

  const IResource &res_;
  bool reset_polarity_;

  unique_ptr<Ports> ports_;
  int addr_width_;
  int data_width_;
};

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_axi_axi_controller_h_
