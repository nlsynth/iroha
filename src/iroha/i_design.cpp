#include "iroha/i_design.h"

#include "design/object_pool.h"
#include "design/resource_class.h"
#include "iroha/logging.h"
#include "iroha/resource_params.h"
#include "iroha/stl_util.h"

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

IArray::IArray(int address_width, const IValueType &data_type,
	       bool is_external, bool is_ram)
  : address_width_(address_width), data_type_(data_type),
    is_external_(is_external), is_ram_(is_ram) {
}

int IArray::GetAddressWidth() const {
  return address_width_;
}

const IValueType &IArray::GetDataType() const {
  return data_type_;
}

bool IArray::IsExternal() const {
  return is_external_;
}

bool IArray::IsRam() const {
  return is_ram_;
}

IResource::IResource(ITable *table, IResourceClass *resource_class)
  : table_(table), resource_class_(resource_class),
    params_(new ResourceParams), id_(-1), array_(nullptr), module_(nullptr) {
  ObjectPool *pool =
    table->GetModule()->GetDesign()->GetObjectPool();
  pool->resources_.Add(this);
  pool->resource_params_.Add(params_);
}

IResource::~IResource() {
  if (array_ != nullptr) {
    delete array_;
  }
}

int IResource::GetId() const {
  return id_;
}

void IResource::SetId(int id) {
  id_ = id;
}

IResourceClass *IResource::GetClass() const {
  return resource_class_;
}

ITable *IResource::GetTable() {
  return table_;
}

ResourceParams *IResource::GetParams() const {
  return params_;
}

IArray *IResource::GetArray() const {
  return array_;
}

void IResource::SetArray(IArray *array) {
  array_ = array;
}

IModule *IResource::GetModule() const {
  return module_;
}

void IResource::SetModule(IModule *module) {
  module_ = module;
}

IValueType::IValueType() : width_(32) {
}

int IValueType::GetWidth() const {
  return width_;
}

void IValueType::SetWidth(int width) {
  width_ = width;
}

IValue::IValue() : value_(0) {
}

IRegister::IRegister(ITable *table, const string &name)
  : table_(table), name_(name), id_(-1),
    has_initial_value_(false), is_const_(false), state_local_(false) {
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

int IRegister::GetId() const {
  return id_;
}

void IRegister::SetId(int id) {
  id_ = id;
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

void IRegister::SetConst(bool c) {
  is_const_ = c;
}

bool IRegister::IsConst() const {
  return is_const_;
}

void IRegister::SetStateLocal(bool s) {
  state_local_ = s;
}

bool IRegister::IsStateLocal() const {
  return state_local_;
}

IInsn::IInsn(IResource *resource) : resource_(resource), id_(-1) {
  IDesign *design =
    resource_->GetTable()->GetModule()->GetDesign();
  design->GetObjectPool()->insns_.Add(this);
}

IResource *IInsn::GetResource() const {
  return resource_;
}

int IInsn::GetId() const {
  return id_;
}

void IInsn::SetId(int id) {
  id_ = id;
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

ITable::ITable(IModule *module)
  : module_(module), id_(-1), initial_state_(nullptr) {
  module->GetDesign()->GetObjectPool()->tables_.Add(this);

  // Add transition resource.
  IResourceClass *tr_class =
    GetTransitionResourceClassFromDesign(module->GetDesign());
  IResource *tr = new IResource(this, tr_class);
  tr->SetId(1);
  resources_.push_back(tr);
}

IModule *ITable::GetModule() {
  return module_;
}

int ITable::GetId() const {
  return id_;
}

void ITable::SetId(int id) {
  id_ = id;
}

void ITable::SetInitialState(IState *state) {
  CHECK(state->GetTable() == this);
  initial_state_ = state;
}

IState *ITable::GetInitialState() const {
  return initial_state_;
}

IModule::IModule(IDesign *design, const string &name)
  : design_(design), name_(name), parent_(nullptr) {
  design->GetObjectPool()->modules_.Add(this);
}

const string &IModule::GetName() const {
  return name_;
}

IDesign *IModule::GetDesign() const {
  return design_;
}

void IModule::SetParentModule(IModule *mod) {
  parent_ = mod;
}

IModule *IModule::GetParentModule() const {
  return parent_;
}

IDesign::IDesign() : objects_(new ObjectPool), params_(new ResourceParams) {
  InstallResourceClasses(this);
}

IDesign::~IDesign() {
  delete objects_;
  delete params_;
}

ObjectPool *IDesign::GetObjectPool() {
  return objects_;
}

ResourceParams *IDesign::GetParams() const{
  return params_;
}

}  // namespace iroha
