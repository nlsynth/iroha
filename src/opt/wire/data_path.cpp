#include "opt/wire/data_path.h"

#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "opt/bb_set.h"
#include "opt/debug_annotation.h"
#include "opt/delay_info.h"

namespace iroha {
namespace opt {
namespace wire {

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
  if (sources_.size() > 0) {
    for (auto &p : sources_) {
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
  map<IRegister *, IInsn *> output_to_insn;
  for (IState *st : bb_->states_) {
    // State local.
    for (IInsn *insn : st->insns_) {
      for (IRegister *oreg : insn->outputs_) {
	if (oreg->IsStateLocal()) {
	  output_to_insn[oreg] = insn;
	}
      }
    }
    // Process inputs.
    for (IInsn *insn : st->insns_) {
      for (IRegister *ireg : insn->inputs_) {
	IInsn *src_insn = output_to_insn[ireg];
	if (src_insn != nullptr) {
	  PathNode *src_node = insn_to_node[src_insn];
	  PathNode *this_node = insn_to_node[insn];
	  this_node->sources_[src_node->GetId()] = src_node;
	}
      }
    }
    // Not state local.
    for (IInsn *insn : st->insns_) {
      for (IRegister *oreg : insn->outputs_) {
	if (!oreg->IsStateLocal()) {
	  output_to_insn[oreg] = insn;
	}
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
  for (auto &p : node->sources_) {
    PathNode *sourceNode = p.second;
    SetAccumlatedDelay(dinfo, sourceNode);
  }
  int maxSourceDelay = 0;
  for (auto &p : node->sources_) {
    PathNode *sourceNode = p.second;
    if (maxSourceDelay < sourceNode->accumlated_delay_) {
      maxSourceDelay = sourceNode->accumlated_delay_;
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

}  // namespace wire
}  // namespace opt
}  // namespace iroha
