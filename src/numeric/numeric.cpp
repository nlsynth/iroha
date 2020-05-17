#include "numeric/numeric.h"

#include "numeric/numeric_manager.h"

#include <iomanip>
#include <sstream>

namespace {
iroha::NumericManager *default_manager;
}  // namespace

namespace iroha {

Numeric::Numeric() {
  value_.value_[0] = 0;
}

void Numeric::SetValue0(uint64_t value) {
  if (type_.IsExtraWide()) {
    value_.extra_wide_value_->value_[0] = value;
  } else {
    value_.value_[0] = value;
  }
}

std::string Numeric::Format() const {
  return Format(type_, value_);
}

std::string Numeric::Format(const NumericWidth &w, const NumericValue &val) {
  if (w.IsExtraWide()) {
    return FormatArray(w, val.extra_wide_value_->value_);
  } else if (w.IsWide()) {
    return FormatArray(w, val.value_);
  } else {
    uint64_t v = val.value_[0];
    std::string s;
    if (w.IsSigned() && (v & (1 << (w.GetWidth() - 1)))) {
      s = "-";
      v = (~v) + 1;
    }
    std::stringstream ss;
    ss << v;
    return s + ss.str();
  }
}

std::string Numeric::FormatArray(const NumericWidth &type,
				 const uint64_t *v) {
  int w = (type.GetWidth() + 63) / 64;
  std::stringstream ss;
  bool first = true;
  for (int i = w - 1; i >= 0; --i) {
    if (!first) {
      ss << "_";
    }
    ss << std::hex << std::setw(16) << std::setfill('0');
    ss << v[i];
    first = false;
  }
  return ss.str();
}


NumericManager *Numeric::DefaultManager() {
  if (default_manager == nullptr) {
    default_manager = new NumericManager;
  }
  return default_manager;
}

NumericManager *Numeric::CreateManager() {
  return new NumericManager;
}

void Numeric::DeleteManager(NumericManager *mgr) {
  delete mgr;
}

void Numeric::CopyValue(const Numeric &src, NumericManager *mgr, Numeric *dst) {
  if (!src.type_.IsExtraWide()) {
    *(dst->GetMutableArray()) = src.GetArray();
    return;
  }
  if (mgr == nullptr) {
    mgr = src.GetArray().extra_wide_value_->owner_;
  }
  mgr->CopyValue(src, dst->GetMutableArray());
}

void Numeric::CopyValueWithWidth(const NumericValue &src_value,
				 const NumericWidth &src_width,
				 const NumericWidth &dst_width,
				 NumericManager *mgr,
				 NumericValue *value) {
  if (mgr == nullptr) {
    mgr = DefaultManager();
  }
  mgr->MayPopulateStorage(dst_width, value);
  if (src_width.IsExtraWide()) {
    if (dst_width.IsExtraWide()) {
      for (int i = 0; i < 32; ++i) {
	value->extra_wide_value_->value_[i] =
	  src_value.extra_wide_value_->value_[i];
      }
    } else {
      for (int i = 0; i < 8; ++i) {
	value->value_[i] =  src_value.extra_wide_value_->value_[i];
      }
    }
  } else {
    if (dst_width.IsExtraWide()) {
      for (int i = 0; i < 8; ++i) {
	value->extra_wide_value_->value_[i] = src_value.value_[i];
      }
      for (int i = 8; i < 32; ++i) {
	value->extra_wide_value_->value_[i] = 0;
      }
    } else {
      *value = src_value;
    }
  }
  if (dst_width.GetWidth() > src_width.GetWidth()) {
    if (dst_width.IsExtraWide()) {
      FixupArray(src_width.GetWidth(), 32, value->extra_wide_value_->value_);
    } else {
      FixupArray(src_width.GetWidth(), 8, value->value_);
    }
  }
}

void Numeric::FixupArray(int w, int l, uint64_t *v) {
  uint64_t mask = ~0;
  mask >>= (64 - (w % 64));
  int value_count;
  if (w == 0) {
    value_count = 1;
  } else {
    value_count = ((w - 1) / 64) + 1;
  }
  for (int i = value_count; i < l; ++i) {
    v[i] = 0;
  }
  v[value_count - 1] &= mask;
}

void Numeric::MayPopulateStorage(const NumericWidth &width,
				 NumericManager *mgr, NumericValue *v) {
  if (mgr == nullptr) {
    mgr = DefaultManager();
  }
  mgr->MayPopulateStorage(width, v);
}

void Numeric::MayExpandStorage(NumericManager *mgr, Numeric *n) {
  if (!n->type_.IsExtraWide()) {
    return;
  }
  NumericValue savedValue = n->GetArray();
  MayPopulateStorage(n->type_, mgr, n->GetMutableArray());
  NumericValue *nv = n->GetMutableArray();
  nv->extra_wide_value_->Clear();
  for (int i = 0; i < 8; ++i) {
    nv->extra_wide_value_->value_[i] = savedValue.value_[i];
  }
}

void Numeric::Clear(const NumericWidth &width, NumericValue *value) {
  if (width.IsExtraWide()) {
    value->extra_wide_value_->Clear();
  } else {
    for (int i = 0; i < 8; ++i) {
      value->value_[i] = 0;
    }
  }
}

}  // namespace iroha
