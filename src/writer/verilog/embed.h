// -*- C++ -*-
#ifndef _writer_verilog_embed_h_
#define _writer_verilog_embed_h_

#include "writer/verilog/common.h"

#include <set>

namespace iroha {
namespace writer {
namespace verilog {

class Embed {
public:
  ~Embed();
  void RequestModule(const ResourceParams &params);
  void BuildModuleInstantiation(const IResource &res,
				const Ports &ports,
				ostream &os);
  // Writes embedded file contents.
  bool Write(ostream &os);

private:
  set<string> files_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_embed_h_
