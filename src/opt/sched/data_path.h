// -*- C++ -*-
#ifndef _opt_sched_data_path_h_
#define _opt_sched_data_path_h_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

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
  map<int, PathNode *> &GetResourceNodeMap(IResource *res);

private:
  void SetAccumlatedDelay(DelayInfo *dinfo, PathNode *node);
  // W->W, W->R
  void BuildFromWrite(map<IInsn *, PathNode *> &insn_to_node);
  // R->W
  void BuildFromRead(map<IInsn *, PathNode *> &insn_to_node);
  void BuildEdge(PathEdgeType type, PathNode *src_node, int reg_index,
		 PathNode *this_node);
  void BuildEdgeForReg(map<IInsn *, PathNode *> &insn_to_node,
		       map<IRegister *, pair<IInsn *, int> > &reg_to_insn,
		       PathEdgeType type, IInsn *insn, IRegister *reg);

  BB *bb_;
  VirtualResourceSet *vrset_;
  // node id (insn id) to PathNode.
  map<int, PathNode *> nodes_;
  set<PathEdge *> edges_;
  // Per resource map from st_index to node.
  map<IResource *, map<int, PathNode *> > resource_node_map_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_data_path_h_
