#include "iroha/resource_params.h"

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
  ResourceParamValue *LookupParam(const string &key,
				  const string &dflt);
  bool GetBoolParam(const string &key,
		    bool dflt);
  void SetBoolParam(const string &key, bool value);
  string GetStringParam(const string &key,
			const string &dflt);
  void SetStringParam(const string &key, const string &value);
  int GetIntParam(const string &key, int dflt);
  void SetIntParam(const string &key, int value);
  vector<ResourceParamValue *> params_;
};

ResourceParamValue *ResourceParamValueSet::LookupParam(const string &key,
						       const string &dflt) {
  for (auto *p : params_) {
    if (p->key_ == key) {
      return p;
    }
  }
  ResourceParamValue *p = nullptr;
  if (!dflt.empty()) {
    p = new ResourceParamValue;
    p->key_ = key;
    p->values_.push_back(dflt);
    params_.push_back(p);
  }
  return p;
}

bool ResourceParamValueSet::GetBoolParam(const string &key,
					 bool dflt) {
  string d = boolToStr(dflt);
  ResourceParamValue *p = LookupParam(key, d);
  if (p->values_.size() > 0 && p->values_[0] == "true") {
    return true;
  }
  return false;
}

void ResourceParamValueSet::SetBoolParam(const string &key, bool value) {
  auto *param = LookupParam(key, boolToStr(value));
  param->values_.clear();
  param->values_.push_back(boolToStr(value));
}

string ResourceParamValueSet::GetStringParam(const string &key,
					     const string &dflt) {
  ResourceParamValue *p = LookupParam(key, dflt);
  if (p->values_.size() > 0) {
    return p->values_[0];
  }
  return string();
}
void ResourceParamValueSet::SetStringParam(const string &key,
					   const string &value) {
  auto *param = LookupParam(key, value);
  param->values_.clear();
  param->values_.push_back(value);
}

int ResourceParamValueSet::GetIntParam(const string &key,
				       int dflt) {
  string a = Util::Itoa(dflt);
  ResourceParamValue *p = LookupParam(key, a);
  if (p->values_.size() > 0) {
    return Util::Atoi(p->values_[0]);
  }
  return dflt;
}

void ResourceParamValueSet::SetIntParam(const string &key, int value) {
  auto *param = LookupParam(key, Util::Itoa(value));
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

bool ResourceParams::GetResetPolarity() const {
  return values_->GetBoolParam(resource::kResetPolarity, false);
}

void ResourceParams::SetResetPolarity(bool p) {
  values_->SetBoolParam(resource::kResetPolarity, p);
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

}  // namespace iroha
