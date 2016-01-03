// -*- C++ -*-
#ifndef _design_object_pool_h_
#define _design_object_pool_h_

#include "iroha/common.h"
#include "iroha/i_design.h"

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
  Pool<IChannel> channels_;
};

}  // namespace iroha

#endif  // _design_object_pool_h_
