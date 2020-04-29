#include "opt/sched/path_node.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"

namespace iroha {
namespace opt {
namespace sched {

PathEdge::PathEdge(int id, PathEdgeType type, PathNode *source_node, PathNode *sink_node,
		   int source_reg_index)
  : id_(id), type_(type), source_node_(source_node), source_reg_index_(source_reg_index),
    sink_node_(sink_node) {
}

int PathEdge::GetId() {
  return id_;
}

PathEdgeType PathEdge::GetType() {
  return type_;
}

bool PathEdge::IsWtoR() {
  return (type_ == WRITE_READ);
}

bool PathEdge::IsWtoW() {
  return (type_ == WRITE_WRITE);
}

bool PathEdge::IsRtoW() {
  return (type_ == READ_WRITE);
}

bool PathEdge::IsDep() {
  return (type_ == INSN_DEP);
}

IRegister *PathEdge::GetSourceReg() {
  CHECK(IsWtoR() || IsWtoW());
  return source_node_->GetInsn()->outputs_[source_reg_index_];
}

void PathEdge::SetSourceReg(IRegister *reg) {
  CHECK(IsWtoR() || IsWtoW());
  source_node_->GetInsn()->outputs_[source_reg_index_] = reg;
}

PathNode *PathEdge::GetSourceNode() {
  return source_node_;
}

int PathEdge::GetSourceRegIndex() {
  return source_reg_index_;
}

PathNode *PathEdge::GetSinkNode() {
  return sink_node_;
}

const char *PathEdge::TypeName(PathEdgeType type) {
  switch (type) {
  case PathEdgeType::WRITE_READ: return "W_R";
  case PathEdgeType::WRITE_WRITE: return "W_W";
  case PathEdgeType::READ_WRITE: return "R_W";
  default:;
  }
  return "INVALID";
}

PathNode::PathNode(BBDataPath *path, int st_index, IInsn *insn, VirtualResource *vres)
  : state_local_delay_(0), path_(path),
    initial_st_index_(st_index), final_st_index_(st_index),
    insn_(insn), vres_(vres), node_delay_(0), accumlated_delay_from_leaf_(0) {
}

int PathNode::GetId() {
  return insn_->GetId();
}

int PathNode::GetInitialStIndex() {
  return initial_st_index_;
}

int PathNode::GetFinalStIndex() {
  return final_st_index_;
}

void PathNode::SetFinalStIndex(int final_st_index) {
  final_st_index_ = final_st_index;
}

VirtualResource *PathNode::GetVirtualResource() {
  return vres_;
}

int PathNode::GetNodeDelay() {
  return node_delay_;
}

void PathNode::SetNodeDelay(int node_delay) {
  node_delay_ = node_delay;
}

int PathNode::GetAccumlatedDelayFromLeaf() {
  return accumlated_delay_from_leaf_;
}

void PathNode::SetAccumlatedDelayFromLeaf(int accumlated_delay_from_leaf) {
  accumlated_delay_from_leaf_ = accumlated_delay_from_leaf;
}

void PathNode::Dump(ostream &os) {
  os << "Node: " << GetId() << "@" << initial_st_index_ << " "
     << node_delay_ << " " << accumlated_delay_from_leaf_ << "\n";
  if (source_edges_.size() > 0) {
    for (auto &p : source_edges_) {
      PathEdge *edge = p.second;
      int source_node_id = edge->GetSourceNode()->GetId();
      os << " " << source_node_id << " " << PathEdge::TypeName(edge->GetType());
    }
    os << "\n";
  }
}

IInsn *PathNode::GetInsn() {
  return insn_;
}

bool PathNode::IsTransition() {
  return resource::IsTransition(*(insn_->GetResource()->GetClass()));
}

BBDataPath *PathNode::GetBBDataPath() {
  return path_;
}

}  // namespace sched
}  // namespace opt
}  // namespace iroha
