// -*- C++ -*-
#ifndef _writer_verilog_channel_h_
#define _writer_verilog_channel_h_

#include "writer/verilog/common.h"
#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class Channel : public Resource {
public:
  Channel(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  static string DataPort(const IChannel &ic);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_channel_h_
