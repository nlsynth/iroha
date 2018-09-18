// -*- C++ -*-
#ifndef _opt_wire_data_path_h_
#define _opt_wire_data_path_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

class PathEdge {
public:
  PathEdge(IInsn *insn);

  int GetId();

  IInsn *insn_;
  map<int, PathEdge *> sources_;
};

class DataPath {
public:
  DataPath(BB *bb);
  ~DataPath();

  void Build();
private:
  BB *bb_;
  map<int, PathEdge *> edges_;
};

class DataPathSet {
public:
  DataPathSet();
  ~DataPathSet();

  void Build(BBSet *bbs);

private:
  BBSet *bbs_;
  map<BB *, DataPath *> data_pathes_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_data_path_h_
