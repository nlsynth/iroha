#include "iroha/iroha.h"

#include "builder/exp_builder.h"
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

}  // namespace iroha
