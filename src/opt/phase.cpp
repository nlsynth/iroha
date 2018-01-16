#include "opt/phase.h"

#include "iroha/i_design.h"
#include "opt/debug_annotation.h"

namespace iroha {
namespace opt {

Phase::Phase() : annotation_(nullptr) {
}

Phase::~Phase() {
}

void Phase::SetName(const string &name) {
  name_ = name;
}

void Phase::SetAnnotation(DebugAnnotation *annotation) {
  annotation_ = annotation;
}

bool Phase::Apply(IDesign *design) {
  OutputPhaseHeader(name_);
  return ApplyForDesign(design);
}

bool Phase::ApplyForDesign(IDesign *design) {
  return ApplyForAllModules("", design);
}

bool Phase::ApplyForAllModules(const string &key, IDesign *design) {
  bool all_ok = true;
  for (auto *mod : design->modules_) {
    all_ok &= ApplyForModule(key, mod);
  }
  return all_ok;
}

bool Phase::ApplyForModule(const string &key, IModule *module) {
  bool all_ok = true;
  for (auto *table : module->tables_) {
    all_ok &= ApplyForTable(key, table);
  }
  return all_ok;
}

bool Phase::ApplyForTable(const string &key, ITable *table) {
  return true;
}

void Phase::OutputPhaseHeader(const string &msg) {
  if (annotation_ == nullptr) {
    return;
  }
  ostream &os = annotation_->GetDumpStream();
  os << "<h1> Phase: " << msg << "</h1>\n";
}

}  // namespace opt
}  // namespace iroha
