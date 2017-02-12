// -*- C++ -*-
#ifndef _writer_verilog_axi_controller_h_
#define _writer_verilog_axi_controller_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

class Controller {
public:
  Controller(const IResource &res, bool reset_polarity);
  ~Controller();

  void Write(ostream &os);

  static string ResetName(bool polarity);

private:
  const IResource &res_;
  bool reset_polarity_;
  unique_ptr<Ports> ports_;
};

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif // _writer_verilog_axi_controller_h_
