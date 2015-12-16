#include "iroha/i_design.h"

#include "design/object_pool.h"
#include "design/resource_class.h"
#include "iroha/logging.h"

namespace iroha {

IResourceClass::IResourceClass(const string &name, bool is_exclusive,
			       IDesign *design)
  : name_(name), design_(design), is_exclusive_(is_exclusive) {
}

IDesign *IResourceClass::GetDesign() {
  return design_;
}

const string &IResourceClass::GetName() const {
  return name_;
}

bool IResourceClass::IsExclusive() {
  return is_exclusive_;
}

IResource::IResource(ITable *table, IResourceClass *resource_class)
  : table_(table), resource_class_(resource_class) {
  table->GetModule()->GetDesign()->GetObjectPool()->resources_.Add(this);
}

IResourceClass *IResource::GetClass() const {
  return resource_class_;
}

ITable *IResource::GetTable() {
  return table_;
}

IRegister::IRegister(ITable *table)
  : initial_value_(0), has_initial_value_(false),
    state_local_(false), table_(table) {
  IDesign *design =
    table->GetModule()->GetDesign();
  design->GetObjectPool()->registers_.Add(this);
}

ITable *IRegister::GetTable() const {
  return table_;
}

IInsn::IInsn(IResource *resource) : resource_(resource) {
  IDesign *design =
    resource_->GetTable()->GetModule()->GetDesign();
  design->GetObjectPool()->insns_.Add(this);
}

IResource *IInsn::GetResource() const {
  return resource_;
}

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

  // Add transition resource.
  IResourceClass *tr_class =
    GetTransitionResourceClassFromDesign(module->GetDesign());
  IResource *tr = new IResource(this, tr_class);
  resources_.push_back(tr);
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
  InstallResourceClasses(this);
}

IDesign::~IDesign() {
  delete objects_;
}

ObjectPool *IDesign::GetObjectPool() {
  return objects_;
}

}  // namespace iroha
