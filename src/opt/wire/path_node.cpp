#include "opt/wire/path_node.h"

#include "iroha/i_design.h"

namespace iroha {
namespace opt {
namespace wire {

PathEdge::PathEdge(int id, PathNode *source_node, PathNode *sink_node,
		   int source_reg_index)
  : source_node_(source_node), sink_node_(sink_node),
    source_reg_index_(source_reg_index), id_(id) {
}

int PathEdge::GetId() {
  return id_;
}

IRegister *PathEdge::GetSourceReg() {
  return source_node_->GetInsn()->outputs_[source_reg_index_];
}

PathNode::PathNode(BBDataPath *path, int st_index, IInsn *insn)
  : node_delay_(0), state_local_delay_(0), accumlated_delay_(0),
    final_st_index_(st_index),
    path_(path),initial_st_index_(st_index), insn_(insn) {
}

int PathNode::GetId() {
  return insn_->GetId();
}

void PathNode::Dump(ostream &os) {
  os << "Node: " << GetId() << "@" << initial_st_index_ << " "
     << node_delay_ << " " << accumlated_delay_ << "\n";
  if (source_edges_.size() > 0) {
    for (auto &p : source_edges_) {
      int source_node_id = p.first;
      os << " " << source_node_id;
    }
    os << "\n";
  }
}

IInsn *PathNode::GetInsn() {
  return insn_;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
