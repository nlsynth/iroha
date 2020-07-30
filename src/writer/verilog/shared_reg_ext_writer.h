// -*- C++ -*-
#ifndef _writer_verilog_shared_reg_ext_writer_h_
#define _writer_verilog_shared_reg_ext_writer_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class SharedRegExtWriter : public Resource {
public:
  SharedRegExtWriter(const IResource &res, const Table &table);
  virtual ~SharedRegExtWriter() {};

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  static void GetAccessorFeatures(const IResource *accessor,
				  bool *use_notify, bool *use_mailbox);
  static bool UseNotify(const IResource *accessor);
  static bool UseMailbox(const IResource *accessor);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_shared_reg_ext_writer_h_
