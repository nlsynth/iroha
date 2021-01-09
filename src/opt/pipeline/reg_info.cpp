#include "opt/pipeline/reg_info.h"

#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "opt/loop/loop_block.h"
#include "opt/optimizer_log.h"

namespace iroha {
namespace opt {
namespace pipeline {

RegInfo::RegInfo(loop::LoopBlock *lb) : lb_(lb) {}

RegInfo::~RegInfo() { STLDeleteSecondElements(&wr_deps_); }

bool RegInfo::BuildWRDep(OptimizerLog *opt_log) {
  map<IRegister *, int> write_pos;
  map<IRegister *, int> last_read_pos;
  int sindex = 0;
  for (IState *st : lb_->GetStates()) {
    for (IInsn *insn : st->insns_) {
      // Reads.
      for (IRegister *reg : insn->inputs_) {
        last_read_pos[reg] = sindex;
      }
      // Writes.
      for (IRegister *reg : insn->outputs_) {
        if (write_pos.find(reg) != write_pos.end()) {
          // write conflicts.
          ostream &os = opt_log->GetDumpStream();
          os << "Give up due to multiple writes<br/>\n";
          return false;
        }
        if (!reg->IsNormal()) {
          continue;
        }
        write_pos[reg] = sindex;
      }
    }
    ++sindex;
  }
  for (auto it : last_read_pos) {
    IRegister *reg = it.first;
    auto jt = write_pos.find(reg);
    if (jt == write_pos.end()) {
      // no write in this loop.
      continue;
    }
    int windex = jt->second;
    int rindex = it.second;
    if (windex < rindex) {
      // Write -> Read.
      WRDep *dep = new WRDep();
      dep->write_lst_index_ = windex;
      dep->read_lst_index_ = rindex;
      wr_deps_[reg] = dep;
    }
  }
  Dump(opt_log);
  return true;
}

void RegInfo::Dump(OptimizerLog *opt_log) {
  if (wr_deps_.size() == 0) {
    return;
  }
  ostream &os = opt_log->GetDumpStream();
  os << "In pipleine register W-R dependencies.<br/>\n";
  for (auto p : wr_deps_) {
    WRDep *d = p.second;
    IRegister *reg = p.first;
    auto &sts = lb_->GetStates();
    os << "r_" << reg->GetId() << " " << reg->GetName()
       << " w:" << sts[d->write_lst_index_]->GetId()
       << " r:" << sts[d->read_lst_index_]->GetId() << "<br/>\n";
  }
}

map<IRegister *, WRDep *> &RegInfo::GetWRDeps() { return wr_deps_; }

WRDep *RegInfo::GetWRDep(IRegister *reg) {
  auto it = wr_deps_.find(reg);
  if (it == wr_deps_.end()) {
    return nullptr;
  }
  return it->second;
}

}  // namespace pipeline
}  // namespace opt
}  // namespace iroha
