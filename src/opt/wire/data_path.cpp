#include "opt/wire/data_path.h"

#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "opt/bb_set.h"
#include "opt/debug_annotation.h"
#include "opt/delay_info.h"
#include "opt/wire/path_node.h"
#include "opt/wire/virtual_resource.h"

namespace iroha {
namespace opt {
namespace wire {

BBDataPath::BBDataPath(BB *bb, VirtualResourceSet *vrset)
  : bb_(bb), vrset_(vrset) {
}

BBDataPath::~BBDataPath() {
  STLDeleteSecondElements(&nodes_);
  STLDeleteValues(&edges_);
}

void BBDataPath::Build() {
  map<IInsn *, PathNode *> insn_to_node;
  int st_index = 0;
  for (IState *st : bb_->states_) {
    for (IInsn *insn : st->insns_) {
      PathNode *n = new PathNode(this, st_index, insn, vrset_->GetFromInsn(insn));
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

void BBDataPath::SetDelay(DelayInfo *dinfo) {
  for (auto &p : nodes_) {
    PathNode *n = p.second;
    n->SetAccumlatedDelayFromLeaf(-1);
    n->SetNodeDelay(dinfo->GetInsnDelay(n->GetInsn()));
  }
  for (auto &p : nodes_) {
    PathNode *n = p.second;
    SetAccumlatedDelay(dinfo, n);
  }
}

void BBDataPath::SetAccumlatedDelay(DelayInfo *dinfo, PathNode *node) {
  if (node->GetAccumlatedDelayFromLeaf() >= 0) {
    return;
  }
  for (auto &p : node->source_edges_) {
    PathNode *source_node = p.second->GetSourceNode();
    SetAccumlatedDelay(dinfo, source_node);
  }
  int max_source_delay = 0;
  for (auto &p : node->source_edges_) {
    PathNode *source_node = p.second->GetSourceNode();
    if (max_source_delay < source_node->GetAccumlatedDelayFromLeaf()) {
      max_source_delay = source_node->GetAccumlatedDelayFromLeaf();
    }
  }
  node->SetAccumlatedDelayFromLeaf(max_source_delay + node->GetNodeDelay());
}

void BBDataPath::Dump(ostream &os) {
  os << "DataPath BB: " << bb_->bb_id_ << "\n";
  for (auto &p : nodes_) {
    p.second->Dump(os);
  }
}

BB *BBDataPath::GetBB() {
  return bb_;
}

map<int, PathNode *> &BBDataPath::GetNodes() {
  return nodes_;
}

DataPathSet::DataPathSet() {
}

DataPathSet::~DataPathSet() {
  STLDeleteSecondElements(&data_paths_);
}

void DataPathSet::Build(BBSet *bset) {
  vres_set_.reset(new VirtualResourceSet(bset->GetTable()));
  bbs_ = bset;
  for (BB *bb : bbs_->bbs_) {
    BBDataPath *dp = new BBDataPath(bb, vres_set_.get());
    data_paths_[bb->bb_id_] = dp;
    dp->Build();
  }
  vres_set_->BuildDefaultBinding();
}

void DataPathSet::SetDelay(DelayInfo *dinfo) {
  for (auto &p : data_paths_) {
    BBDataPath *dp = p.second;
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

map<int, BBDataPath *> &DataPathSet::GetPaths() {
  return data_paths_;
}

BBSet *DataPathSet::GetBBSet() {
  return bbs_;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
