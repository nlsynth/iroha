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

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  static string RegName(const IResource &reg);
  static string WriterName(const IResource &writer);
  static string WriterEnName(const IResource &writer);
  // Notifier
  //  notification wire to readers
  static string RegNotifierName(const IResource &reg);
  //  notification from each writer
  static string WriterNotifierName(const IResource &writer);
  // Mailbox
  static string RegMailboxName(const IResource &reg);
  static string RegMailboxPutReqName(const IResource &writer);
  static string RegMailboxPutAckName(const IResource &writer);
  static string RegMailboxGetReqName(const IResource &reader);
  static string RegMailboxGetAckName(const IResource &reader);

  static void AddCommonRootWire(const IModule *common_root, const Table *tab,
				const IResource *accessor, bool is_write);
  // Generates wiring in a sub module declaration .abc(abc)
  static void AddChildWire(const IResource *accessor, bool is_write, bool use_notify, bool use_mailbox, ostream &os);

private:
  void BuildReadWire();
  void AddReadPort(const IModule *imod, const IResource *reader, bool upward);
  void GetOptions(bool *use_notify, bool *use_mailbox);
  void BuildMailbox();

  int width_;
  bool has_default_output_value_;
  int default_output_value_;
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
