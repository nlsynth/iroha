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
  Controller(const IResource &res,
	     bool reset_polarity);
  ~Controller();

  void Write(ostream &os);

  static void AddPorts(Module *mod, bool r, bool w,
		       string *s);

  static string ResetName(bool polarity);

private:
  static void GenReadChannel(Module *module, Ports *ports,
			     string *s);
  static void GenWriteChannel(Module *module, Ports *ports,
			      string *s);
  static void AddPort(const string &name, int width, bool dir,
		      Module *module, Ports *ports, string *s);
  void OutputFsm(ostream &os);
  void ReaderFsm(ostream &os);
  void WriterFsm(ostream &os);

  const IResource &res_;
  bool reset_polarity_;
  unique_ptr<Ports> ports_;
  bool r_, w_;
  int addr_width_;
  int data_width_;
};

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif // _writer_verilog_axi_controller_h_
