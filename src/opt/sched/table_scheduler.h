// -*- C++ -*-
#ifndef _opt_sched_table_scheduler_h_
#define _opt_sched_table_scheduler_h_

#include "opt/sched/common.h"

namespace iroha {
namespace opt {
namespace sched {

class TableScheduler {
 public:
  TableScheduler(ITable *table, DelayInfo *delay_info, OptimizerLog *opt_log);
  virtual ~TableScheduler();
  bool Perform();

 private:
  void IterateScheduling();

  ITable *table_;
  DelayInfo *delay_info_;
  OptimizerLog *opt_log_;
  unique_ptr<BBSet> bset_;

  unique_ptr<DataPathSet> data_path_set_;
};

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_scheduler_h_
