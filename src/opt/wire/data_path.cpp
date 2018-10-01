#include "opt/wire/data_path.h"

#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "opt/bb_set.h"
#include "opt/debug_annotation.h"
#include "opt/delay_info.h"

namespace iroha {
namespace opt {
namespace wire {

PathEdge::PathEdge(DataPath *path, int st_index, IInsn *insn)
  : path_(path), edge_delay_(0), state_local_delay_(0), accumlated_delay_(0),
    initial_st_index_(st_index), final_st_index_(st_index),
    insn_(insn) {
}

int PathEdge::GetId() {
  return insn_->GetId();
}

void PathEdge::Dump(ostream &os) {
  os << "Edge: " << GetId() << "@" << initial_st_index_ << " "
     << edge_delay_ << " " << accumlated_delay_ << "\n";
  if (sources_.size() > 0) {
    for (auto &p : sources_) {
      int source_edge_id = p.first;
      os << " " << source_edge_id;
    }
    os << "\n";
  }
}

DataPath::DataPath(BB *bb) : bb_(bb) {
}

DataPath::~DataPath() {
  STLDeleteSecondElements(&edges_);
}

void DataPath::Build() {
  map<IInsn *, PathEdge *> insn_to_edge;
  int st_index = 0;
  for (IState *st : bb_->states_) {
    for (IInsn *insn : st->insns_) {
      PathEdge *e = new PathEdge(this, st_index, insn);
      edges_[e->GetId()] = e;
      insn_to_edge[insn] = e;
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
	  PathEdge *src_edge = insn_to_edge[src_insn];
	  PathEdge *this_edge = insn_to_edge[insn];
	  this_edge->sources_[src_edge->GetId()] = src_edge;
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
  for (auto &p : edges_) {
    PathEdge *e = p.second;
    e->accumlated_delay_ = -1;
    e->edge_delay_ = dinfo->GetInsnDelay(e->insn_);
  }
  for (auto &p : edges_) {
    PathEdge *e = p.second;
    SetAccumlatedDelay(dinfo, e);
  }
}

void DataPath::SetAccumlatedDelay(DelayInfo *dinfo, PathEdge *edge) {
  if (edge->accumlated_delay_ >= 0) {
    return;
  }
  for (auto &p : edge->sources_) {
    PathEdge *sourceEdge = p.second;
    SetAccumlatedDelay(dinfo, sourceEdge);
  }
  int maxSourceDelay = 0;
  for (auto &p : edge->sources_) {
    PathEdge *sourceEdge = p.second;
    if (maxSourceDelay < sourceEdge->accumlated_delay_) {
      maxSourceDelay = sourceEdge->accumlated_delay_;
    }
  }
  edge->accumlated_delay_ = maxSourceDelay + edge->edge_delay_;
}

void DataPath::Dump(ostream &os) {
  os << "DataPath BB: " << bb_->bb_id_ << "\n";
  for (auto &p : edges_) {
    p.second->Dump(os);
  }
}

BB *DataPath::GetBB() {
  return bb_;
}

map<int, PathEdge *> &DataPath::GetEdges() {
  return edges_;
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
