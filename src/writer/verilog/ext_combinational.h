// -*- C++ -*-
#ifndef _writer_verilog_ext_combinational_h_
#define _writer_verilog_ext_combinational_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class ExtCombinational : public Resource {
public:
  ExtCombinational(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;
  virtual void CollectNames(Names *names) override;

private:
  void AddPort(const string &name, const string &wire_name,
	       int width, string *connection);

  static string ArgPin(const IResource *res, int nth);
  static string RetPin(const IResource *res, int nth);
  static string PinName(const IResource *res, const string &name, int nth);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_ext_combinational_h_
