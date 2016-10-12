#include "iroha/i_design.h"

#include "design/object_pool.h"
#include "design/design_util.h"
#include "iroha/opt_api.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "iroha/stl_util.h"
#include "iroha/writer_api.h"
#include "opt/debug_annotation.h"

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

IArrayImage::IArrayImage(IDesign *design) : design_(design), id_(-1) {
  design->GetObjectPool()->array_images_.Add(this);
}

IDesign *IArrayImage::GetDesign() const {
  return design_;
}

int IArrayImage::GetId() const {
  return id_;
}

void IArrayImage::SetId(int id) {
  id_ = id;
}

const string &IArrayImage::GetName() const {
  return name_;
}

void IArrayImage::SetName(const string &name) {
  name_ = name;
}

IArray::IArray(int address_width, const IValueType &data_type,
	       bool is_external, bool is_ram)
  : address_width_(address_width), data_type_(data_type),
    is_external_(is_external), is_ram_(is_ram), array_image_(nullptr) {
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

void IArray::SetArrayImage(IArrayImage *image) {
  array_image_ = image;
}

IArrayImage *IArray::GetArrayImage() const {
  return array_image_;
}

IResource::IResource(ITable *table, IResourceClass *resource_class)
  : table_(table), resource_class_(resource_class),
    params_(new ResourceParams), id_(-1), array_(nullptr),
    callee_table_(nullptr), foreign_register_(nullptr),
    channel_(nullptr), shared_reg_(nullptr) {
  ObjectPool *pool =
    table->GetModule()->GetDesign()->GetObjectPool();
  pool->resources_.Add(this);
  pool->resource_params_.Add(params_);
}

IResource::~IResource() {
  delete array_;
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

ITable *IResource::GetTable() const {
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

ITable *IResource::GetCalleeTable() const {
  return callee_table_;
}

void IResource::SetCalleeTable(ITable *table) {
  callee_table_ = table;
}

void IResource::SetForeignRegister(IRegister *reg) {
  foreign_register_ = reg;
}

IRegister *IResource::GetForeignRegister() const{
  return foreign_register_;
}

void IResource::SetChannel(IChannel *ch) {
  channel_ = ch;
}

IChannel *IResource::GetChannel() const {
  return channel_;
}

IResource *IResource::GetSharedReg() const {
  return shared_reg_;
}

void IResource::SetSharedReg(IResource *res) {
  shared_reg_ = res;
}

IChannel::IChannel(IDesign *design)
  : design_(design), id_(-1), writer_(nullptr), reader_(nullptr),
    params_(new ResourceParams) {
  design_->GetObjectPool()->channels_.Add(this);
}

IDesign *IChannel::GetDesign() const {
  return design_;
}

int IChannel::GetId() const {
  return id_;
}

void IChannel::SetId(int id) {
  id_ = id;
}

const IValueType &IChannel::GetValueType() const {
  return value_type_;
}

void IChannel::SetValueType(const IValueType &value_type) {
  value_type_ = value_type;
}

void IChannel::SetWriter(IResource *res) {
  writer_ = res;
  res->SetChannel(this);
}

void IChannel::SetReader(IResource *res) {
  reader_ = res;
  res->SetChannel(this);
}

IResource *IChannel::GetWriter() const {
  return writer_;
}

IResource *IChannel::GetReader() const {
  return reader_;
}

ResourceParams *IChannel::GetParams() const {
  return params_;
}

IValueType::IValueType() : width_(32), is_signed_(false) {
}

int IValueType::GetWidth() const {
  return width_;
}

void IValueType::SetWidth(int width) {
  width_ = width;
}

bool IValueType::IsSigned() const {
  return is_signed_;
}

void IValueType::SetIsSigned(bool is_signed) {
  is_signed_ = is_signed;
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

void IRegister::SetName(const string &name) {
  name_ = name;
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

bool IRegister::IsNormal() const {
  return !state_local_ && !is_const_;
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

const string &IInsn::GetOperand() const {
  return operand_;
}

void IInsn::SetOperand(const string &opr) {
  operand_ = opr;
}

IState::IState(ITable *table) : table_(table), id_(-1) {
  table->GetModule()->GetDesign()->GetObjectPool()->states_.Add(this);
}

ITable *IState::GetTable() const {
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

  // Add transition resource for sure.
  IResourceClass *tr_class =
    DesignUtil::GetTransitionResourceClassFromDesign(module->GetDesign());
  IResource *tr = new IResource(this, tr_class);
  // Tentative id not to duplicate with user input. User may or may not
  // overwrite.
  tr->SetId(-1);
  resources_.push_back(tr);
}

IModule *ITable::GetModule() const {
  return module_;
}

int ITable::GetId() const {
  return id_;
}

void ITable::SetId(int id) {
  id_ = id;
}

string ITable::GetName() const {
  return name_;
}

void ITable::SetName(const string &name) {
  name_ = name;
}

void ITable::SetInitialState(IState *state) {
  CHECK(state->GetTable() == this);
  initial_state_ = state;
}

IState *ITable::GetInitialState() const {
  return initial_state_;
}

IModule::IModule(IDesign *design, const string &name)
  : design_(design), id_(-1), name_(name), parent_(nullptr),
    params_(new ResourceParams) {
  design->GetObjectPool()->modules_.Add(this);
}

int IModule::GetId() const {
  return id_;
}

void IModule::SetId(int id) {
  id_ = id;
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

ResourceParams *IModule::GetParams() const{
  return params_;
}

IDesign::IDesign()
  : objects_(new ObjectPool), params_(new ResourceParams), annotation_(nullptr) {
  resource::InstallResourceClasses(this);
}

IDesign::~IDesign() {
  delete objects_;
  delete params_;
  delete annotation_;
}

ObjectPool *IDesign::GetObjectPool() {
  return objects_;
}

ResourceParams *IDesign::GetParams() const{
  return params_;
}

void IDesign::SetDebugAnnotation(opt::DebugAnnotation *annotation) {
  delete annotation_;
  annotation_ = annotation;
}
  
opt::DebugAnnotation *IDesign::GetDebugAnnotation() const {
  return annotation_;
}

OptAPI *IDesign::GetOptAPI() const {
  return opt_api_.get();
}

void IDesign::SetOptAPI(OptAPI *opt_api) {
  opt_api_.reset(opt_api);
}

WriterAPI *IDesign::GetWriterAPI() const {
  return writer_api_.get();
}

void IDesign::SetWriterAPI(WriterAPI *writer_api) {
  writer_api_.reset(writer_api);
}

}  // namespace iroha
