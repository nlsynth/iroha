#include "opt/pass.h"

#include "iroha/i_design.h"
#include "opt/debug_annotation.h"

namespace iroha {
namespace opt {

Pass::Pass() : optimizer_(nullptr), annotation_(nullptr) {}

Pass::~Pass() {}

void Pass::SetName(const string &name) { name_ = name; }

void Pass::SetOptimizer(Optimizer *opt) { optimizer_ = opt; }

void Pass::SetAnnotation(DebugAnnotation *annotation) {
  annotation_ = annotation;
}

bool Pass::Apply(IDesign *design) {
  OutputPhaseHeader(name_);
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

void Pass::OutputPhaseHeader(const string &msg) {
  if (!annotation_->IsEnabled()) {
    return;
  }
  ostream &os = annotation_->GetDumpStream();
  os << "<h1> Phase: " << msg << "</h1>\n";
}

}  // namespace opt
}  // namespace iroha
