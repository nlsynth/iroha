// -*- C++ -*-
#ifndef _writer_verilog_embed_h_
#define _writer_verilog_embed_h_

#include "writer/verilog/common.h"
#include "writer/verilog/resource.h"

#include <set>

namespace iroha {
namespace writer {
namespace verilog {

class EmbeddedModules {
public:
  ~EmbeddedModules();
  void RequestModule(const ResourceParams &params);
  InternalSRAM *RequestInternalSRAM(const Module &mod,
				    const IResource &res,
				    int num_ports);
  // Called from MasterPort::BuildResource()
  void RequestAxiMasterController(const IResource *axi_port,
				  bool reset_polarity);
  // Called from SlavePort::BuildResource()
  void RequestAxiSlaveController(const IResource *axi_port,
				 bool reset_polarity);

  // Writes embedded file contents.
  bool Write(ostream &os);

private:
  set<string> files_;
  vector<InternalSRAM *> srams_;
  vector<pair<const IResource *, bool> > axi_ports_;
};

class EmbeddedResource : public Resource {
public:
  EmbeddedResource(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

private:
  string ArgRegName(const ResourceParams &params, int nth);
  string AckWireName(const ResourceParams &params);
  string ReqRegName(const ResourceParams &params);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_embed_h_
