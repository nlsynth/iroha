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
				    const IResource &res);

  // Writes embedded file contents.
  bool Write(ostream &os);

private:
  set<string> files_;
  vector<InternalSRAM *> srams_;
};

class EmbeddedResource : public Resource {
public:
  EmbeddedResource(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_embed_h_
