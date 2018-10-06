#include "opt/wire/data_path.h"

#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "opt/bb_set.h"
#include "opt/debug_annotation.h"
#include "opt/delay_info.h"

namespace iroha {
namespace opt {
namespace wire {

PathEdge::PathEdge(int id, PathNode *source_node, PathNode *sink_node, int source_reg_index)
  : source_node_(source_node), sink_node_(sink_node), source_reg_index_(source_reg_index), id_(id) {
}

int PathEdge::GetId() {
  return id_;
}

IRegister *PathEdge::GetSourceReg() {
  return source_node_->insn_->outputs_[source_reg_index_];
}

PathNode::PathNode(DataPath *path, int st_index, IInsn *insn)
  : path_(path), node_delay_(0), state_local_delay_(0), accumlated_delay_(0),
    initial_st_index_(st_index), final_st_index_(st_index),
    insn_(insn) {
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

DataPath::DataPath(BB *bb) : bb_(bb) {
}

DataPath::~DataPath() {
  STLDeleteSecondElements(&nodes_);
  STLDeleteValues(&edges_);
}

void DataPath::Build() {
  map<IInsn *, PathNode *> insn_to_node;
  int st_index = 0;
  for (IState *st : bb_->states_) {
    for (IInsn *insn : st->insns_) {
      PathNode *n = new PathNode(this, st_index, insn);
      nodes_[n->GetId()] = n;
      insn_to_node[insn] = n;
    }
    ++st_index;
  }
  // insn and the index in outputs_[].
  map<IRegister *, pair<IInsn *, int> > output_to_insn;
  for (IState *st : bb_->states_) {
    // State local.
    for (IInsn *insn : st->insns_) {
      int oindex = 0;
      for (IRegister *oreg : insn->outputs_) {
	if (oreg->IsStateLocal()) {
	  output_to_insn[oreg] = make_pair(insn, oindex);
	}
	++oindex;
      }
    }
    // Process inputs.
    for (IInsn *insn : st->insns_) {
      for (IRegister *ireg : insn->inputs_) {
	auto p = output_to_insn[ireg];
	IInsn *src_insn = p.first;
	int oindex = p.second;
	if (src_insn != nullptr) {
	  PathNode *src_node = insn_to_node[src_insn];
	  PathNode *this_node = insn_to_node[insn];
	  // Use current number of edge as the edge id.
	  int edge_id = edges_.size() + 1;
	  PathEdge *edge = new PathEdge(edge_id, src_node, this_node, oindex);
	  edges_.insert(edge);
	  this_node->source_edges_[src_node->GetId()] = edge;
	  src_node->sink_edges_[edge->GetId()] = edge;
	}
      }
    }
    // Not state local.
    for (IInsn *insn : st->insns_) {
      int oindex = 0;
      for (IRegister *oreg : insn->outputs_) {
	if (!oreg->IsStateLocal()) {
	  output_to_insn[oreg] = make_pair(insn, oindex);
	}
	++oindex;
      }
    }
  }
}

void DataPath::SetDelay(DelayInfo *dinfo) {
  for (auto &p : nodes_) {
    PathNode *n = p.second;
    n->accumlated_delay_ = -1;
    n->node_delay_ = dinfo->GetInsnDelay(n->insn_);
  }
  for (auto &p : nodes_) {
    PathNode *n = p.second;
    SetAccumlatedDelay(dinfo, n);
  }
}

void DataPath::SetAccumlatedDelay(DelayInfo *dinfo, PathNode *node) {
  if (node->accumlated_delay_ >= 0) {
    return;
  }
  for (auto &p : node->source_edges_) {
    PathNode *source_node = p.second->source_node_;
    SetAccumlatedDelay(dinfo, source_node);
  }
  int maxSourceDelay = 0;
  for (auto &p : node->source_edges_) {
    PathNode *source_node = p.second->source_node_;
    if (maxSourceDelay < source_node->accumlated_delay_) {
      maxSourceDelay = source_node->accumlated_delay_;
    }
  }
  node->accumlated_delay_ = maxSourceDelay + node->node_delay_;
}

void DataPath::Dump(ostream &os) {
  os << "DataPath BB: " << bb_->bb_id_ << "\n";
  for (auto &p : nodes_) {
    p.second->Dump(os);
  }
}

BB *DataPath::GetBB() {
  return bb_;
}

map<int, PathNode *> &DataPath::GetNodes() {
  return nodes_;
}

DataPathSet::DataPathSet() {
}

DataPathSet::~DataPathSet() {
  STLDeleteSecondElements(&data_paths_);
}

void DataPathSet::Build(BBSet *bset) {
  bbs_ = bset;
  for (BB *bb : bbs_->bbs_) {
    DataPath *dp = new DataPath(bb);
    data_paths_[bb->bb_id_] = dp;
    dp->Build();
  }
}

void DataPathSet::SetDelay(DelayInfo *dinfo) {
  for (auto &p : data_paths_) {
    DataPath *dp = p.second;
    dp->SetDelay(dinfo);
  }
}

void DataPathSet::Dump(DebugAnnotation *an) {
  ostream &os = an->GetDumpStream();
  os << "DataPathSet table: " << bbs_->GetTable()->GetId() << "\n";
  for (auto &p : data_paths_) {
    p.second->Dump(os);
  }
}

map<int, DataPath *> &DataPathSet::GetPaths() {
  return data_paths_;
}

BBSet *DataPathSet::GetBBSet() {
  return bbs_;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
