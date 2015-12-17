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

IValueType::IValueType() : width_(32), is_enum_(false) {
}

int IValueType::GetWidth() {
  return width_;
}

bool IValueType::IsEnum() {
  return is_enum_;
}

void IValueType::SetIsEnum(bool is_enum) {
  is_enum_ = is_enum;
}

void IValueType::SetWidth(int width) {
  width_ = width;
}

IValue::IValue() : value_(0) {
}

IRegister::IRegister(ITable *table, const string &name)
  : state_local_(false), is_const_(false), table_(table),
    name_(name), has_initial_value_(false) {
  IDesign *design =
    table->GetModule()->GetDesign();
  design->GetObjectPool()->registers_.Add(this);
}

ITable *IRegister::GetTable() const {
  return table_;
}

const string &IRegister::GetName() const {
  return name_;
}

void IRegister::SetInitialValue(IValue &value) {
  has_initial_value_ = true;
  initial_value_ = value;
  value_type_ = value.type_;
}

const IValue &IRegister::GetInitialValue() const {
  return initial_value_;
}

bool IRegister::HasInitialValue() const {
  return has_initial_value_;
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
