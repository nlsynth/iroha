// -*- C++ -*-
#ifndef _writer_verilog_resource_h_
#define _writer_verilog_resource_h_

#include "writer/cxx/common.h"

namespace iroha {
namespace writer {
namespace cxx {

class Resource {
public:
  static void BuildResource(IResource *res, ClassWriter *cw);
  static void WriteInsn(IInsn *insn, ostream &os);
  static string RegValue(IRegister *reg);
  static string MemName(IResource *res);

private:
  static void WriteBinOp(IInsn *insn, ostream &os);
  static void WriteUniOp(IInsn *insn, ostream &os);
  static void WriteBitShift(IInsn *insn, ostream &os);
  static void WritePrint(IInsn *insn, ostream &os);
  static void WriteAssert(IInsn *insn, ostream &os);
  static void WriteMapped(IInsn *insn, ostream &os);
};

}  // namespace cxx
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_resource_h_
