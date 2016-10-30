#include "writer/cxx/cxx_writer.h"

#include "design/design_util.h"
#include "writer/cxx/module.h"

namespace iroha {
namespace writer {
namespace cxx {

CxxWriter::CxxWriter(const IDesign *design, ostream &os)
  : design_(design), os_(os) {
}

void CxxWriter::Write() {
  os_ << "// Generated from " << PACKAGE << "-" << VERSION << ".\n\n";
  IModule *root = DesignUtil::GetRootModule(design_);

  Module root_mod(root);

  root_mod.Build();

  os_ << "#include \"cxx-rt.h\"\n\n"
      << "using namespace iroha_rt;\n\n"
      << "namespace iroha_design {\n\n";

  root_mod.Write(os_);

  os_ << "}  // namespace iroha_design\n\n"
      << "int main(int argc, char **argv) {\n"
      << "  iroha_design::" << root_mod.GetName() << " _r;\n"
      << "  iroha_rt::Runner runner(&_r);\n"
      << "  runner.Run();\n"
      << "  return 0;\n"
      << "}\n";
}

}  // namespace cxx
}  // namespace writer
}  // namespace iroha
