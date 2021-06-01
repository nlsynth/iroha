//  -*- C++ -*-
#ifndef _writer_connection_h_
#define _writer_connection_h_

#include <map>
#include <set>

#include "iroha/common.h"

namespace iroha {
namespace writer {

// Keyed by owner (parent) resource.
class AccessorInfo {
 public:
  vector<IResource *> task_callers_;
  vector<IResource *> shared_memory_port0_accessors_;
  // resources accesses secondary port (port1) of this memory resource.
  vector<IResource *> shared_memory_port1_accessors_;
  vector<IResource *> shared_reg_readers_;
  vector<IResource *> shared_reg_writers_;
  vector<IResource *> shared_reg_ext_writers_;
  // dataflowin-s attached to this shared register resource.
  vector<IResource *> shared_reg_children_;
  vector<IResource *> fifo_readers_;
  vector<IResource *> fifo_writers_;
  vector<IResource *> ext_input_accessors_;
  vector<IResource *> ext_output_accessors_;
  vector<IResource *> ticker_accessors_;
  vector<IResource *> study_accessors_;
};

class Connection {
 public:
  Connection(const IDesign *design);
  ~Connection();

  void Build();

  const AccessorInfo *GetAccessorInfo(const IResource *res) const;
  const vector<IResource *> &GetTaskCallers(const IResource *res) const;
  const vector<IResource *> &GetSharedRegWriters(const IResource *res) const;
  const vector<IResource *> &GetSharedRegExtWriters(const IResource *res) const;
  const vector<IResource *> &GetSharedRegReaders(const IResource *res) const;
  const vector<IResource *> &GetSharedRegChildren(const IResource *res) const;
  const vector<IResource *> &GetSharedMemoryPort0Accessors(
      const IResource *res) const;
  const vector<IResource *> &GetSharedMemoryPort1Accessors(
      const IResource *res) const;
  const vector<IResource *> &GetFifoWriters(const IResource *res) const;
  const vector<IResource *> &GetFifoReaders(const IResource *res) const;
  const vector<IResource *> &GetExtInputAccessors(const IResource *res) const;
  const vector<IResource *> &GetExtOutputAccessors(const IResource *res) const;
  const vector<IResource *> &GetTickerAccessors(const IResource *res) const;
  const vector<IResource *> &GetStudyAccessors(const IResource *res) const;

  static const IModule *GetCommonRoot(const IModule *m1, const IModule *m2);

 private:
  const vector<IResource *> *GetResourceVector(
      const map<const IResource *, vector<IResource *>> &m,
      const IResource *res) const;
  void ProcessTable(ITable *tab);
  void ProcessSharedRegAccessors(ITable *tab);
  void ProcessSharedMemoryAccessors(ITable *tab);
  void ProcessFifoAccessors(ITable *tab);
  void ProcessExtIOAccessors(ITable *tab);
  void ProcessTickerAccessors(ITable *tab);
  void ProcessStudyAccessors(ITable *tab);
  void AssignMemoryAccessorPort(AccessorInfo *ainfo);
  AccessorInfo *FindAccessorInfo(const IResource *res);

  const IDesign *design_;
  map<const IResource *, AccessorInfo *> accessors_;
  AccessorInfo empty_accessors_;
};

}  // namespace writer
}  // namespace iroha

#endif  // _writer_connection_h_
