#include "writer/verilog/shared_memory.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/module_template.h"
#include "writer/verilog/embed.h"
#include "writer/verilog/internal_sram.h"
#include "writer/verilog/module.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {

SharedMemory::SharedMemory(const IResource &res, const Table &table)
  : Resource(res, table) {
}

void SharedMemory::BuildResource() {
  auto *klass = res_.GetClass();
  if (resource::IsSharedMemory(*klass)) {
    BuildMemoryResource();
  }
  if (resource::IsSharedMemoryReader(*klass)) {
    BuildMemoryReaderResource();
  }
  if (resource::IsSharedMemoryWriter(*klass)) {
    BuildMemoryWriterResource();
  }
}

void SharedMemory::BuildMemoryResource() {
  InternalSRAM *sram =
    tab_.GetEmbeddedModules()->RequestInternalSRAM(*tab_.GetModule(), res_);
  ostream &es = tmpl_->GetStream(kEmbeddedInstanceSection);
  string name = sram->GetModuleName();
  string res_id = Util::Itoa(res_.GetId());
  string inst = name + "_inst_" + res_id;
  es << "  " << name << " " << inst << "("
     <<");\n";
}

void SharedMemory::BuildMemoryReaderResource() {
}

void SharedMemory::BuildMemoryWriterResource() {
}

void SharedMemory::BuildInsn(IInsn *insn, State *st) {
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
