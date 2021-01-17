// -*- C++ -*-
// -*- C++ -*-
#ifndef _writer_verilog_debug_resource_h_
#define _writer_verilog_debug_resource_h_

#include "writer/verilog/common.h"
#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

// print and assert
class DebugResource : public Resource {
 public:
  DebugResource(const IResource &res, const Table &table);
  virtual ~DebugResource(){};

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;
  virtual void CollectNames(Names *names) override;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_debug_resource_h_
