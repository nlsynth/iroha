#include "iroha/i_design.h"

#include "design/object_pool.h"
#include "iroha/logging.h"

namespace iroha {

IState::IState(ITable *table) : table_(table), id_(-1) {
  table->GetModule()->GetDesign()->GetObjectPool()->states_.Add(this);
}

ITable *IState::GetTable() {
  return table_;
}

int IState::GetId() const {
  return id_;
}

void IState::SetId(int id) {
  id_ = id;
}

ITable::ITable(IModule *module) : module_(module), initial_state_(nullptr) {
  module->GetDesign()->GetObjectPool()->tables_.Add(this);
}

IModule *ITable::GetModule() {
  return module_;
}

void ITable::SetInitialState(IState *state) {
  CHECK(state->GetTable() == this);
  initial_state_ = state;
}

IState *ITable::GetInitialState() {
  return initial_state_;
}

IModule::IModule(IDesign *design, const string &name)
  : design_(design), name_(name) {
  design->GetObjectPool()->modules_.Add(this);
}

const string &IModule::GetName() const {
  return name_;
}

IDesign *IModule::GetDesign() {
  return design_;
}

IDesign::IDesign() : objects_(new ObjectPool) {
}

IDesign::~IDesign() {
  delete objects_;
}

ObjectPool *IDesign::GetObjectPool() {
  return objects_;
}

}  // namespace iroha
