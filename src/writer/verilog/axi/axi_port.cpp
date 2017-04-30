#include "writer/verilog/axi/axi_port.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

AxiPort::AxiPort(const IResource &res, const Table &table)
  : Resource(res, table) {
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

