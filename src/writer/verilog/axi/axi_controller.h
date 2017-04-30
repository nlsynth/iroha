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

  static string ResetName(bool polarity);

protected:
  static void AddPort(const string &name, int width, bool dir_s2m,
		      Module *module, Ports *ports, string *s);
  static void GenReadChannel(Module *module, Ports *ports,
			     string *s);
  static void GenWriteChannel(Module *module, Ports *ports,
			      string *s);

  const IResource &res_;
  bool reset_polarity_;
};

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_axi_axi_controller_h_
