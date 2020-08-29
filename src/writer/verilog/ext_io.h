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
  virtual ~ExtIO() {};

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;
  virtual void CollectNames(Names *names) override;

  static string InputRegName(const IResource &res);
  static string OutputRegName(const IResource &res);
  static string BufRegNameWithDistance(const string &port, int distance, int stage);

private:
  void BuildExtInputInsn(IInsn *insn, State *st);
  void BuildExtOutputInsn(IInsn *insn, State *st);
  void BuildPeekExtOutputInsn(IInsn *insn);
  void BuildExtInputResource();
  void BuildExtOutputResource();
  void BuildExtOutputAccessor();
  void BuildBufRegChain(const string &port, int width);
  string BufRegName(const string &output_port, int stage);
  string InputWireName(const string &input_port);
  string PrevInputRegName(const string &input_port);

  bool has_default_output_value_;
  int default_output_value_;
  bool has_accessor_output_;
  int distance_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_ext_io_h_
