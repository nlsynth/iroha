// -*- C++ -*-
#ifndef _opt_wire_data_path_h_
#define _opt_wire_data_path_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

// Data path graph for a BB.
class BBDataPath {
public:
  BBDataPath(BB *bb, VirtualResourceSet *vrset);
  ~BBDataPath();

  void Build();
  void SetDelay(DelayInfo *dinfo);
  void Dump(ostream &os);
  BB *GetBB();
  map<int, PathNode *> &GetNodes();

private:
  void SetAccumlatedDelay(DelayInfo *dinfo, PathNode *node);

  BB *bb_;
  VirtualResourceSet *vrset_;
  // node id (insn id) to PathNode.
  map<int, PathNode *> nodes_;
  set<PathEdge *> edges_;
};

class DataPathSet {
public:
  DataPathSet();
  ~DataPathSet();

  void Build(BBSet *bbs);
  void SetDelay(DelayInfo *dinfo);
  void Dump(DebugAnnotation *an);
  map<int, BBDataPath *> &GetPaths();
  BBSet *GetBBSet();
  VirtualResourceSet *GetVirtualResourceSet();

private:
  BBSet *bbs_;
  // bb_id to BBDataPath.
  map<int, BBDataPath *> data_paths_;
  std::unique_ptr<VirtualResourceSet> vres_set_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_data_path_h_
