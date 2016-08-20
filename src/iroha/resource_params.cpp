#include "iroha/resource_params.h"

#include "iroha/logging.h"
#include "iroha/stl_util.h"

static const string boolToStr(bool b) {
  if (b) {
    return "true";
  } else {
    return "false";
  }
}

namespace iroha {
namespace resource {
class ResourceParamValue {
public:
  string key_;
  vector <string> values_;
};

class ResourceParamValueSet {
public:
  ~ResourceParamValueSet() {
    STLDeleteValues(&params_);
  }
  ResourceParamValue *GetParam(const string &key,
			       bool cr,
			       const string &dflt);
  ResourceParamValue *LookupParam(const string &key) const;
  vector<string> GetParamKeys() const;
  vector<string> GetValues(const string &key) const;
  void SetValues(const string &key, const vector<string> &values);
  bool GetBoolParam(const string &key,
		    bool dflt) const;
  void SetBoolParam(const string &key, bool value);
  string GetStringParam(const string &key,
			const string &dflt) const;
  void SetStringParam(const string &key, const string &value);
  int GetIntParam(const string &key, int dflt) const;
  void SetIntParam(const string &key, int value);

  vector<ResourceParamValue *> params_;
};

ResourceParamValue *ResourceParamValueSet::GetParam(const string &key,
						    bool cr,
						    const string &dflt) {
  ResourceParamValue *p = LookupParam(key);
  if (p != nullptr) {
    return p;
  }
  if (!cr) {
    return nullptr;
  }
  p = new ResourceParamValue;
  p->key_ = key;
  if (!dflt.empty()) {
    p->values_.push_back(dflt);
  }
  params_.push_back(p);
  return p;
}

ResourceParamValue *ResourceParamValueSet::LookupParam(const string &key)
  const {
  for (auto *p : params_) {
    if (p->key_ == key) {
      return p;
    }
  }
  return nullptr;
}

vector<string> ResourceParamValueSet::GetParamKeys() const {
  vector<string> v;
  for (auto *p : params_) {
    v.push_back(p->key_);
  }
  return v;
}

vector<string> ResourceParamValueSet::GetValues(const string &key) const {
  for (auto *p : params_) {
    if (p->key_ == key) {
      return p->values_;
    }
  }
  vector<string> v;
  return v;
}

void ResourceParamValueSet::SetValues(const string &key,
				      const vector<string> &values) {
  auto *param = GetParam(key, true, "");
  param->values_ = values;
}

bool ResourceParamValueSet::GetBoolParam(const string &key,
					 bool dflt) const {
  ResourceParamValue *p = LookupParam(key);
  if (p == nullptr) {
    return dflt;
  }
  if (p->values_.size() > 0 && Util::ToLower(p->values_[0]) == "true") {
    return true;
  }
  return false;
}

void ResourceParamValueSet::SetBoolParam(const string &key, bool value) {
  auto *param = GetParam(key, true, boolToStr(value));
  param->values_.clear();
  param->values_.push_back(boolToStr(value));
}

string ResourceParamValueSet::GetStringParam(const string &key,
					     const string &dflt) const {
  ResourceParamValue *p = LookupParam(key);
  if (p == nullptr) {
    return dflt;
  }
  if (p->values_.size() > 0) {
    return p->values_[0];
  }
  return string();
}

void ResourceParamValueSet::SetStringParam(const string &key,
					   const string &value) {
  auto *param = GetParam(key, true, "");
  if (param == nullptr) {
    CHECK(value.empty()) << "param should be non null if value is not empty";
    return;
  }
  param->values_.clear();
  param->values_.push_back(value);
}

int ResourceParamValueSet::GetIntParam(const string &key,
				       int dflt) const {
  ResourceParamValue *p = LookupParam(key);
  if (p == nullptr) {
    return dflt;
  }
  if (p->values_.size() > 0) {
    return Util::Atoi(p->values_[0]);
  }
  return dflt;
}

void ResourceParamValueSet::SetIntParam(const string &key, int value) {
  auto *param = GetParam(key, true, Util::Itoa(value));
  param->values_.clear();
  param->values_.push_back(Util::Itoa(value));
}

}  // namespace resource

ResourceParams::ResourceParams()
  : values_(new resource::ResourceParamValueSet) {
}

ResourceParams::~ResourceParams() {
  delete values_;
}

vector<string> ResourceParams::GetParamKeys() const {
  return values_->GetParamKeys();
}

vector<string> ResourceParams::GetValues(const string &key) const {
  return values_->GetValues(key);
}

void ResourceParams::SetValues(const string &key,
			       const vector<string> &values) {
  values_->SetValues(key, values);
}

string ResourceParams::GetModuleNamePrefix() const {
  return values_->GetStringParam(resource::kModuleNamePrefix, "");
}

void ResourceParams::SetModuleNamePrefix(const string &prefix) {
  values_->SetStringParam(resource::kModuleNamePrefix, prefix);
}

string ResourceParams::GetMappedName() const {
  return values_->GetStringParam(resource::kMappedName, "");
}

void ResourceParams::SetMappedName(const string &name) {
  values_->SetStringParam(resource::kMappedName, name);
}

bool ResourceParams::GetResetPolarity() const {
  // default is negative.
  return values_->GetBoolParam(resource::kResetPolarity, false);
}

void ResourceParams::SetResetPolarity(bool p) {
  values_->SetBoolParam(resource::kResetPolarity, p);
}

bool ResourceParams::HasResetPolarity() const {
  return (values_->LookupParam(resource::kResetPolarity) != nullptr);
}

string ResourceParams::GetResetName() const {
  return values_->GetStringParam(resource::kResetName, "");
}

void ResourceParams::SetResetName(const string &name) {
  values_->SetStringParam(resource::kResetName, name);
}

void ResourceParams::SetExtInputPort(const string &input, int width) {
  values_->SetStringParam(resource::kExtInputPort, input);
  values_->SetIntParam(resource::kExtIOWidth, width);
}

void ResourceParams::GetExtInputPort(string *name, int *width) {
  *name = values_->GetStringParam(resource::kExtInputPort, "");
  *width = values_->GetIntParam(resource::kExtIOWidth, 0);
}

void ResourceParams::SetExtOutputPort(const string &output, int width) {
  values_->SetStringParam(resource::kExtOutputPort, output);
  values_->SetIntParam(resource::kExtIOWidth, width);
}

void ResourceParams::GetExtOutputPort(string *name, int *width) {
  *name = values_->GetStringParam(resource::kExtOutputPort, "");
  *width = values_->GetIntParam(resource::kExtIOWidth, 0);
}

void ResourceParams::SetEmbeddedModuleName(const string &mod,
					   const string &fn) {
  values_->SetStringParam(resource::kEmbeddedModule, mod);
  values_->SetStringParam(resource::kEmbeddedModuleFile, fn);
}

string ResourceParams::GetEmbeddedModuleName() const {
  return values_->GetStringParam(resource::kEmbeddedModule, "");
}

string ResourceParams::GetEmbeddedModuleFileName() const {
  return values_->GetStringParam(resource::kEmbeddedModuleFile, "");
}

string ResourceParams::GetEmbeddedModuleClk() const {
  return values_->GetStringParam(resource::kEmbeddedModuleClk, "clk");
}

string ResourceParams::GetEmbeddedModuleReset() const {
  return values_->GetStringParam(resource::kEmbeddedModuleReset, "rst");
}

string ResourceParams::GetEmbeddedModuleReq() const {
  return values_->GetStringParam(resource::kEmbeddedModuleReq, "");
}

string ResourceParams::GetEmbeddedModuleAck() const {
  return values_->GetStringParam(resource::kEmbeddedModuleAck, "");
}

vector<string> ResourceParams::GetEmbeddedModuleArgs() const {
  return values_->GetValues(resource::kEmbeddedModuleArgs);
}

}  // namespace iroha
