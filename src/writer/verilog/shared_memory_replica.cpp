#include "writer/verilog/shared_memory_replica.h"

namespace iroha {
namespace writer {
namespace verilog {

SharedMemoryReplica::SharedMemoryReplica(const IResource &res,
					 const Table &table)
  : Resource(res, table) {
}

void SharedMemoryReplica::BuildResource() {
}

void SharedMemoryReplica::BuildInsn(IInsn *insn, State *st) {
}

void SharedMemoryReplica::CollectNames(Names *names) {
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
