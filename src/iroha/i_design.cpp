#include "iroha/i_design.h"

#include "design/design_util.h"
#include "iroha/i_platform.h"
#include "iroha/logging.h"
#include "iroha/object_pool.h"
#include "iroha/opt_api.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "iroha/stl_util.h"
#include "iroha/writer_api.h"
#include "opt/optimizer_log.h"

namespace iroha {

IResourceClass::IResourceClass(const string &name, bool is_exclusive,
                               IDesign *design)
    : name_(name), design_(design), is_exclusive_(is_exclusive) {}

IDesign *IResourceClass::GetDesign() { return design_; }

const string &IResourceClass::GetName() const { return name_; }

bool IResourceClass::IsExclusive() { return is_exclusive_; }

IArrayImage::IArrayImage(IDesign *design) : design_(design), id_(-1) {
  design->GetObjectPool()->array_images_.Add(this);
}

IDesign *IArrayImage::GetDesign() const { return design_; }

int IArrayImage::GetId() const { return id_; }

void IArrayImage::SetId(int id) { id_ = id; }

const string &IArrayImage::GetName() const { return name_; }

void IArrayImage::SetName(const string &name) { name_ = name; }

IArray::IArray(IResource *res, int address_width, const IValueType &data_type,
               bool is_external, bool is_ram)
    : res_(res),
      address_width_(address_width),
      data_type_(data_type),
      is_external_(is_external),
      is_ram_(is_ram),
      array_image_(nullptr) {
  if (res == nullptr) {
    return;
  }
  auto *pool = res->GetTable()->GetModule()->GetDesign()->GetObjectPool();
  pool->arrays_.Add(this);
}

IResource *IArray::GetResource() const { return res_; }

int IArray::GetAddressWidth() const { return address_width_; }

const IValueType &IArray::GetDataType() const { return data_type_; }

bool IArray::IsExternal() const { return is_external_; }

void IArray::SetExternal(bool is_external) { is_external_ = is_external; }

bool IArray::IsRam() const { return is_ram_; }

void IArray::SetArrayImage(IArrayImage *image) { array_image_ = image; }

IArrayImage *IArray::GetArrayImage() const { return array_image_; }

IResource::IResource(ITable *table, IResourceClass *resource_class)
    : table_(table),
      resource_class_(resource_class),
      params_(new ResourceParams),
      id_(-1),
      array_(nullptr),
      callee_table_(nullptr),
      parent_resource_(nullptr) {
  ObjectPool *pool = table->GetModule()->GetDesign()->GetObjectPool();
  pool->resources_.Add(this);
  pool->resource_params_.Add(params_);
}

IResource::~IResource() {}

int IResource::GetId() const { return id_; }

void IResource::SetId(int id) { id_ = id; }

IResourceClass *IResource::GetClass() const { return resource_class_; }

ITable *IResource::GetTable() const { return table_; }

ResourceParams *IResource::GetParams() const { return params_; }

IArray *IResource::GetArray() const { return array_; }

void IResource::SetArray(IArray *array) { array_ = array; }

ITable *IResource::GetCalleeTable() const { return callee_table_; }

void IResource::SetCalleeTable(ITable *table) { callee_table_ = table; }

IResource *IResource::GetParentResource() const { return parent_resource_; }

void IResource::SetParentResource(IResource *res) { parent_resource_ = res; }

IValueType IValueType::FromNumericWidth(const NumericWidth &w) {
  IValueType iv;
  iv.SetWidth(w.GetWidth());
  iv.SetIsSigned(w.IsSigned());
  return iv;
}

IRegister::IRegister(ITable *table, const string &name)
    : table_(table),
      name_(name),
      id_(-1),
      has_initial_value_(false),
      is_const_(false),
      state_local_(false),
      params_(nullptr) {
  IDesign *design = table->GetModule()->GetDesign();
  design->GetObjectPool()->registers_.Add(this);
}

ITable *IRegister::GetTable() const { return table_; }

const string &IRegister::GetName() const { return name_; }

void IRegister::SetName(const string &name) { name_ = name; }

int IRegister::GetId() const { return id_; }

void IRegister::SetId(int id) { id_ = id; }

void IRegister::SetInitialValue(Numeric &value) {
  has_initial_value_ = true;
  initial_value_ = value;
  value_type_ = IValueType::FromNumericWidth(value.type_);
}

void IRegister::ClearInitialValue() { has_initial_value_ = false; }

const Numeric &IRegister::GetInitialValue() const { return initial_value_; }

bool IRegister::HasInitialValue() const { return has_initial_value_; }

void IRegister::SetConst(bool c) { is_const_ = c; }

bool IRegister::IsConst() const { return is_const_; }

void IRegister::SetStateLocal(bool s) { state_local_ = s; }

bool IRegister::IsStateLocal() const { return state_local_; }

bool IRegister::IsNormal() const { return !state_local_ && !is_const_; }

ResourceParams *IRegister::GetMutableParams(bool cr) {
  if (params_.get() == nullptr && cr) {
    params_.reset(new ResourceParams);
  }
  return params_.get();
}

ResourceParams *IRegister::GetParams() const { return params_.get(); }

IInsn::IInsn(IResource *resource) : resource_(resource), id_(-1) {
  IDesign *design = resource_->GetTable()->GetModule()->GetDesign();
  design->GetObjectPool()->insns_.Add(this);
}

IResource *IInsn::GetResource() const { return resource_; }

void IInsn::SetResource(IResource *resource) { resource_ = resource; }

int IInsn::GetId() const { return id_; }

void IInsn::SetId(int id) { id_ = id; }

const string &IInsn::GetOperand() const { return operand_; }

void IInsn::SetOperand(const string &opr) { operand_ = opr; }

IProfile::IProfile() : valid_(false), raw_count_(0), normalized_count_(0) {}

IState::IState(ITable *table) : table_(table), id_(-1) {
  table->GetModule()->GetDesign()->GetObjectPool()->states_.Add(this);
}

ITable *IState::GetTable() const { return table_; }

int IState::GetId() const { return id_; }

void IState::SetId(int id) { id_ = id; }

IProfile *IState::GetMutableProfile() { return &profile_; }

const IProfile &IState::GetProfile() const { return profile_; }

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

IModule *ITable::GetModule() const { return module_; }

int ITable::GetId() const { return id_; }

void ITable::SetId(int id) { id_ = id; }

string ITable::GetName() const { return name_; }

void ITable::SetName(const string &name) { name_ = name; }

void ITable::SetInitialState(IState *state) {
  CHECK(state->GetTable() == this);
  initial_state_ = state;
}

IState *ITable::GetInitialState() const { return initial_state_; }

IModule::IModule(IDesign *design, const string &name)
    : design_(design),
      id_(-1),
      name_(name),
      parent_(nullptr),
      params_(new ResourceParams) {
  design->GetObjectPool()->modules_.Add(this);
}

int IModule::GetId() const { return id_; }

void IModule::SetId(int id) { id_ = id; }

const string &IModule::GetName() const { return name_; }

IDesign *IModule::GetDesign() const { return design_; }

void IModule::SetParentModule(IModule *mod) { parent_ = mod; }

IModule *IModule::GetParentModule() const { return parent_; }

ResourceParams *IModule::GetParams() const { return params_.get(); }

IDesign::IDesign()
    : objects_(new ObjectPool), params_(new ResourceParams), opt_log_(nullptr) {
  resource::InstallResourceClasses(this);
}

IDesign::~IDesign() {
  delete objects_;
  delete params_;
  delete opt_log_;
}

ObjectPool *IDesign::GetObjectPool() { return objects_; }

ResourceParams *IDesign::GetParams() const { return params_; }

void IDesign::SetOptimizerLog(opt::OptimizerLog *opt_log) {
  delete opt_log_;
  opt_log_ = opt_log;
}

opt::OptimizerLog *IDesign::GetOptimizerLog() const { return opt_log_; }

OptAPI *IDesign::GetOptAPI() const { return opt_api_.get(); }

void IDesign::SetOptAPI(OptAPI *opt_api) { opt_api_.reset(opt_api); }

WriterAPI *IDesign::GetWriterAPI() const { return writer_api_.get(); }

void IDesign::SetWriterAPI(WriterAPI *writer_api) {
  writer_api_.reset(writer_api);
}

void IDesign::ReleasePlatform(IPlatform *platform) {
  platform->SetDesign(nullptr);
}

void IDesign::ManagePlatform(IPlatform *platform) { platform->SetDesign(this); }

}  // namespace iroha
