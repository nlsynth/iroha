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
  int st_index_;
  int edge_delay_;
  int accumlated_delay_;
  IInsn *insn_;
  map<int, PathEdge *> sources_;
};

class DataPath {
public:
  DataPath(BB *bb);
  ~DataPath();

  void Build();
  void Dump(ostream &os);

private:
  BB *bb_;
  map<int, PathEdge *> edges_;
};

class DataPathSet {
public:
  DataPathSet();
  ~DataPathSet();

  void Build(BBSet *bbs);
  void SetLatency(LatencyInfo *lat);
  void Dump(DebugAnnotation *an);

private:
  BBSet *bbs_;
  map<int, DataPath *> data_pathes_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_data_path_h_
