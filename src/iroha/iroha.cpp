#include "iroha/iroha.h"

#include "builder/exp_builder.h"
#include "design/design_tool.h"
#include "writer/writer.h"

namespace iroha {

void Iroha::Init() {
}

IDesign *Iroha::ReadDesignFromFile(const string &fn) {
  return iroha::ExpBuilder::ReadDesign(fn);
}

WriterAPI *Iroha::CreateWriter(const IDesign *design) {
  return new Writer(design);
}

DesignToolAPI *Iroha::CreateDesignTool(IDesign *design) {
  return new DesignTool(design);
}

}  // namespace iroha
