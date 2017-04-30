#include "writer/verilog/axi/slave_controller.h"

#include "writer/verilog/axi/slave_port.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {  

SlaveController::SlaveController(const IResource &res,
				 bool reset_polarity)
  : res_(res), reset_polarity_(reset_polarity) {
}

SlaveController::~SlaveController() {
}

void SlaveController::Write(ostream &os) {
  os << "// slave controller: "
     << SlavePort::ControllerName(res_, reset_polarity_) << "\n";
}
  
}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
