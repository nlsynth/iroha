#include "opt/pass.h"

#include "iroha/i_design.h"
#include "opt/optimizer_log.h"

namespace iroha {
namespace opt {

Pass::Pass() : optimizer_(nullptr), opt_log_(nullptr) {}

Pass::~Pass() {}

void Pass::SetName(const string &name) { name_ = name; }

void Pass::SetOptimizer(Optimizer *opt) { optimizer_ = opt; }

void Pass::SetAnnotation(OptimizerLog *opt_log) { opt_log_ = opt_log; }

bool Pass::Apply(IDesign *design) {
  OutputPassHeader(name_);
  return ApplyForDesign(design);
}

bool Pass::ApplyForDesign(IDesign *design) {
  return ApplyForAllModules("", design);
}

bool Pass::ApplyForAllModules(const string &key, IDesign *design) {
  bool all_ok = true;
  for (auto *mod : design->modules_) {
    all_ok &= ApplyForModule(key, mod);
  }
  return all_ok;
}

bool Pass::ApplyForModule(const string &key, IModule *module) {
  bool all_ok = true;
  for (auto *table : module->tables_) {
    all_ok &= ApplyForTable(key, table);
  }
  return all_ok;
}

bool Pass::ApplyForTable(const string &key, ITable *table) { return true; }

void Pass::OutputPassHeader(const string &msg) {
  if (!opt_log_->IsEnabled()) {
    return;
  }
  ostream &os = opt_log_->GetDumpStream();
  os << "<h1> Pass: " << msg << "</h1>\n";
}

}  // namespace opt
}  // namespace iroha
