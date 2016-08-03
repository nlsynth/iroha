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

void Iroha::SetImportPaths(const vector<string> &paths) {
  Util::SetImportPaths(paths);
}

IDesign *Iroha::ReadDesignFromFile(const string &fn) {
  return builder::ExpBuilder::ReadDesign(fn);
}

WriterAPI *Iroha::CreateWriter(IDesign *design) {
  WriterAPI *writer = design->GetWriterAPI();
  if (writer == nullptr) {
    writer = new writer::Writer(design);
    design->SetWriterAPI(writer);
  }
  return writer;
}

OptAPI *Iroha::CreateOptimizer(IDesign *design) {
  OptAPI *opt = design->GetOptAPI();
  if (opt == nullptr) {
    opt = new opt::Optimizer(design);
    design->SetOptAPI(opt);
  }
  return opt;
}

}  // namespace iroha
