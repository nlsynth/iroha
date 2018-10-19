// -*- C++ -*-
#ifndef _iroha_object_pool_h_
#define _iroha_object_pool_h_

#include "iroha/common.h"

namespace iroha {

template<class T>
class Pool {
public:
  ~Pool() {
    for (int i = 0; i < ptrs_.size(); ++i) {
      delete ptrs_[i];
    }
  }
  void Add(T *p) {
    ptrs_.push_back(p);
  }
  void Release(T *p) {
    std::vector<T *> tmp_ptrs_;
    for (auto *t : ptrs_) {
      if (t != p) {
	tmp_ptrs_.push_back(t);
      }
    }
    ptrs_ = tmp_ptrs_;
  }
  std::vector<T *> ptrs_;
};

class ObjectPool {
public:
  Pool<IModule> modules_;
  Pool<ITable> tables_;
  Pool<IState> states_;
  Pool<IResource> resources_;
  Pool<IResourceClass> resource_classes_;
  Pool<IRegister> registers_;
  Pool<IInsn> insns_;
  Pool<ResourceParams> resource_params_;
  Pool<IArray> arrays_;
  Pool<IArrayImage> array_images_;
  Pool<IPlatform> platforms_;
  Pool<platform::DefNode> def_nodes_;
  Pool<platform::Definition> definitions_;
};

}  // namespace iroha

#endif  // _iroha_object_pool_h_
