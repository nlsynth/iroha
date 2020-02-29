// -*- C++ -*-
#ifndef _writer_verilog_shared_memory_replica_h_
#define _writer_verilog_shared_memory_replica_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class SharedMemoryReplica : public Resource {
public:
  SharedMemoryReplica(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;
  virtual void CollectNames(Names *names) override;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_shared_memory_replica_h_
