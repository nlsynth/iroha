// -*- C++ -*-
#ifndef _opt_wire_path_node_h_
#define _opt_wire_path_node_h_

#include "opt/wire/common.h"

namespace iroha {
namespace opt {
namespace wire {

// Data path (BBDataPath) is a graph comprised of nodes and edges.
//
// ------------                    ------
// |Node(Insn)|-->Edge(Register)-->|Node|
// ------------                    ------

// Represents an IRegister flows from its source insn to sink.
// There are 3 types of dependencies we have to care.
// (1) Write -> Read
//     This is a real dependency and considered to compute latencies.
// (2) Write -> Write
//     This is for ordering. (this might be resolved by renaming the preceding output).
// (3) Read -> Write
//     This is for ordeing.
enum PathEdgeType : int {
  WRITE_READ,
  WRITE_WRITE,
  READ_WRITE,
};
class PathEdge {
public:
  PathEdge(int id, PathEdgeType type, PathNode *source_node, PathNode *sink_node,
	   int source_reg_index);

  int GetId();
  PathEdgeType GetType();
  bool IsWtoR();
  bool IsWtoW();
  bool IsRtoW();
  IRegister *GetSourceReg();
  void SetSourceReg(IRegister *reg);
  int GetSourceRegIndex();
  PathNode *GetSourceNode();
  PathNode *GetSinkNode();
  static const char *TypeName(PathEdgeType type);

private:
  int id_;
  PathEdgeType type_;
  PathNode *source_node_;
  int source_reg_index_;
  PathNode *sink_node_;
};

// Represents an IInsn and connected to other insns via PathEdge-s.
class PathNode {
public:
  PathNode(BBDataPath *path, int st_index, IInsn *insn, VirtualResource *vres);

  int GetId();
  void Dump(ostream &os);
  IInsn *GetInsn();
  int GetInitialStIndex();
  int GetFinalStIndex();
  void SetFinalStIndex(int final_st_index);
  VirtualResource *GetVirtualResource();
  int GetNodeDelay();
  void SetNodeDelay(int node_delay);
  int GetAccumlatedDelayFromLeaf();
  void SetAccumlatedDelayFromLeaf(int accumlated_delay_from_leaf);
  bool IsTransition();
  BBDataPath *GetBBDataPath();

  // Scratch variable for Scheduler.
  int state_local_delay_;
  // key-ed by edge id.
  map<int, PathEdge *> source_edges_;
  // key-ed by edge id.
  map<int, PathEdge *> sink_edges_;

private:
  BBDataPath *path_;
  int initial_st_index_;
  int final_st_index_;
  IInsn *insn_;
  VirtualResource *vres_;
  // Latency of just this node (=insn) (Schedule independent).
  int node_delay_;
  // Maximum accumlated delay from leafs including the delay of this node
  // (Schedule independent).
  int accumlated_delay_from_leaf_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_path_node_h_
