// -*- C++ -*-
#ifndef _writer_verilog_embedded_modules_h_
#define _writer_verilog_embedded_modules_h_

#include "writer/verilog/common.h"
#include "writer/verilog/resource.h"

#include <map>

namespace iroha {
namespace writer {
namespace verilog {

class EmbeddedModules {
public:
  ~EmbeddedModules();

  void RequestModule(const ResourceParams &params);
  // Creates copy of the array.
  InternalSRAM *RequestInternalSRAM(const Module &mod,
				    const IArray &arr,
				    int num_ports);
  // Called from MasterPort::BuildResource()
  void RequestAxiMasterController(const IResource *axi_port);
  // Called from SlavePort::BuildResource()
  void RequestAxiSlaveController(const IResource *axi_port);

  void RequestWireMux(const wire::WireSet *wire_set);

  // Writes embedded file contents.
  bool Write(bool reset_polarity, ostream &os);

private:
  bool CopyFile(const string &fn, const string &rst,
		bool reset_polarity, ostream &os);

  // file name to reset pin name.
  map<string, string> files_;
  vector<InternalSRAM *> srams_;
  vector<const IResource *> axi_ports_;
  vector<const wire::WireSet *> wire_sets_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_embedded_modules_h_
