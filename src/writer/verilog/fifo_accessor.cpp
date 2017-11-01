#include "writer/verilog/fifo_accessor.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/verilog/fifo.h"

namespace iroha {
namespace writer {
namespace verilog {

FifoAccessor::FifoAccessor(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void FifoAccessor::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsFifoReader(*klass)) {
    BuildReader();
  }
  if (resource::IsFifoWriter(*klass)) {
    BuildWriter();
  }
}

void FifoAccessor::BuildInsn(IInsn *insn, State *st) {
}

void FifoAccessor::BuildReader() {
}

void FifoAccessor::BuildWriter() {
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
