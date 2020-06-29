// -*- C++ -*-
#ifndef _writer_verilog_ext_io_h_
#define _writer_verilog_ext_io_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class ExtIO : public Resource {
public:
  ExtIO(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;
  virtual void CollectNames(Names *names) override;

  static string InputRegName(const IResource &res);
  static string BufRegNameWithDistance(const string &port, int distance,
				       int stage);

private:
  void BuildExtInputInsn(IInsn *insn);
  void BuildExtOutputInsn(IInsn *insn, State *st);
  void BuildPeekExtOutputInsn(IInsn *insn);
  void BuildExtInputResource();
  void BuildExtOutputResource();
  void BuildBufRegChain(const string &port, int width);
  string BufRegName(const string &output_port, int stage);

  bool has_default_output_value_;
  int default_output_value_;
  int distance_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_ext_io_h_
