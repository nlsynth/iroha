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

  static string RegName(const IResource &res);
  static string WriterName(const IResource &res);
  static string WriterEnName(const IResource &res);
  static void AddWire(const IModule *common_root, const Table *tab,
		      const IResource *accessor, bool is_write);
  static void AddChildWire(const IResource *res, bool is_write, ostream &os);

private:
  void BuildReadWire();
  void AddReadPort(const IModule *imod, const IResource *reader, bool upward);

  int width_;
  bool has_default_output_value_;
  int default_output_value_;
  const vector<IResource *> *writers_;
  bool need_write_arbitration_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_shared_reg_h_
