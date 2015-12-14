#include "iroha/i_design.h"

#include "design/object_pool.h"

namespace iroha {

IState::IState(ITable *table) : table_(table) {
  table->module_->design_->objects_->states_.Add(this);
}

ITable::ITable(IModule *module) : module_(module) {
  module->design_->objects_->tables_.Add(this);
}

IModule::IModule(IDesign *design, const string &name)
  : design_(design), name_(name) {
  design->objects_->modules_.Add(this);
}

IDesign::IDesign() : objects_(new ObjectPool) {
}

IDesign::~IDesign() {
  delete objects_;
}

}  // namespace iroha
