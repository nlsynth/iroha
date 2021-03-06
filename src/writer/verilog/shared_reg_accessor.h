// -*- C++ -*-
#ifndef _writer_verilog_shared_reg_accessor_h_
#define _writer_verilog_shared_reg_accessor_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class SharedRegAccessor : public Resource {
public:
  SharedRegAccessor(const IResource &res, const Table &table);
  virtual ~SharedRegAccessor() {};

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  static void GetAccessorFeatures(const IResource *accessor,
				  bool *use_notify, bool *use_mailbox);
  static bool UseNotify(const IResource *accessor);
  static bool UseMailbox(const IResource *accessor);

private:
  void BuildSharedRegWriterResource();
  void BuildSharedRegReaderResource();
  void BuildReadInsn(IInsn *insn, State *st);
  void BuildWriteInsn(IInsn *insn, State *st);

  string GetName();

  int width_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_shared_reg_accessor_h_
