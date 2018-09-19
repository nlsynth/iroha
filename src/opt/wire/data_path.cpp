#include "opt/wire/data_path.h"

#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "opt/bb_set.h"
#include "opt/debug_annotation.h"

namespace iroha {
namespace opt {
namespace wire {

PathEdge::PathEdge(IInsn *insn) : insn_(insn) {
}

int PathEdge::GetId() {
  return insn_->GetId();
}

void PathEdge::Dump(ostream &os) {
  os << "Edge: " << GetId() << "\n";
}

DataPath::DataPath(BB *bb) : bb_(bb) {
}

DataPath::~DataPath() {
  STLDeleteSecondElements(&edges_);
}

void DataPath::Build() {
  map<IInsn *, PathEdge *> insn_to_edge;
  for (IState *st : bb_->states_) {
    for (IInsn *insn : st->insns_) {
      PathEdge *e = new PathEdge(insn);
      edges_[e->GetId()] = e;
      insn_to_edge[insn] = e;
    }
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

void DataPath::Dump(ostream &os) {
  os << "DataPath BB: " << bb_->bb_id_ << "\n";
  for (auto &p : edges_) {
    p.second->Dump(os);
  }
}

DataPathSet::DataPathSet() {
}

DataPathSet::~DataPathSet() {
  STLDeleteSecondElements(&data_pathes_);
}

void DataPathSet::Build(BBSet *bset) {
  bbs_ = bset;
  for (BB *bb : bbs_->bbs_) {
    DataPath *dp = new DataPath(bb);
    data_pathes_[bb->bb_id_] = dp;
    dp->Build();
  }
}

void DataPathSet::Dump(DebugAnnotation *an) {
  ostream &os = an->GetDumpStream();
  os << "DataPathSet table: " << bbs_->GetTable()->GetId() << "\n";
  for (auto &p : data_pathes_) {
    p.second->Dump(os);
  }
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
