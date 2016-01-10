#include "iroha/iroha.h"

#include "builder/exp_builder.h"
#include "design/design_tool.h"
#include "opt/optimizer.h"
#include "writer/writer.h"

namespace iroha {

OptAPI::~OptAPI() {
}

WriterAPI::~WriterAPI() {
}

void Iroha::Init() {
  opt::Optimizer::Init();
}

IDesign *Iroha::ReadDesignFromFile(const string &fn) {
  return iroha::ExpBuilder::ReadDesign(fn);
}

WriterAPI *Iroha::CreateWriter(const IDesign *design) {
  return new Writer(design);
}

DesignTool *Iroha::CreateDesignTool(IDesign *design) {
  return new DesignTool(design);
}

OptAPI *Iroha::CreateOptimizer(IDesign *design) {
  return new opt::Optimizer(design);
}

}  // namespace iroha
