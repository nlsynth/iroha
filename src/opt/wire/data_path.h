// -*- C++ -*-
#ifndef _opt_wire_data_path_h_
#define _opt_wire_data_path_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

class PathEdge {
public:
  PathEdge(DataPath *path, int st_index, IInsn *insn);

  int GetId();
  void Dump(ostream &os);

  DataPath *path_;
  int initial_st_index_;
  int final_st_index_;
  int edge_delay_;
  int state_local_delay_;
  int accumlated_delay_;
  IInsn *insn_;
  map<int, PathEdge *> sources_;
};

class DataPath {
public:
  DataPath(BB *bb);
  ~DataPath();

  void Build();
  void SetDelay(DelayInfo *dinfo);
  void Dump(ostream &os);
  BB *GetBB();
  map<int, PathEdge *> &GetEdges();

private:
  void SetAccumlatedDelay(DelayInfo *dinfo, PathEdge *edge);

  BB *bb_;
  // edge id (insn id) to PathEdge.
  map<int, PathEdge *> edges_;
};

class DataPathSet {
public:
  DataPathSet();
  ~DataPathSet();

  void Build(BBSet *bbs);
  void SetDelay(DelayInfo *dinfo);
  void Dump(DebugAnnotation *an);
  map<int, DataPath *> &GetPaths();

private:
  BBSet *bbs_;
  // bb_id to DataPath.
  map<int, DataPath *> data_paths_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_data_path_h_
