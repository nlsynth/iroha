// -*- C++ -*-
#ifndef _writer_verilog_shared_reg_h_
#define _writer_verilog_shared_reg_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class SharedReg : public Resource {
public:
  SharedReg(const IResource &res, const Table &table);
  virtual ~SharedReg() {};

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  // Actual storage (reg).
  static string RegName(const IResource &reg);
  // Notifier
  static string RegNotifierName(const IResource &reg);
  // Resource name.
  static string GetName(const IResource &reg);
  static string GetNameRW(const IResource &reg, bool is_write);
  // Mailbox
  static string RegMailboxName(const IResource &reg);
  static string RegMailboxPutAckName(const IResource &writer);
  static string RegMailboxGetAckName(const IResource &reader);
  // Either notifier or mailbox.
  static string RegMailboxBufName(const IResource &reader);

private:
  void BuildAccessorWireR();
  void BuildAccessorWireW();
  void GetOptions(bool *use_notify, bool *use_mailbox);
  void BuildMailbox();
  void BuildNotifier();

  int width_;
  bool has_default_value_;
  int default_value_;
  // readers_ includes dataflow-in for pipeline.
  vector<IResource *> readers_;
  vector<IResource *> writers_;
  bool need_write_arbitration_;
  bool use_notify_;
  bool use_mailbox_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_shared_reg_h_
