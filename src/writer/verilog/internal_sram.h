// -*- C++ -*-
#ifndef _writer_verilog_internal_sram_h_
#define _writer_verilog_internal_sram_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

class InternalSRAM {
public:
  InternalSRAM(const Module &mod, const IResource &res);

  void Write(ostream &os);

  string GetModuleName() const;
  string GetResetPinName() const;
  const IResource &GetResource() const;

  string AddressWidthSpec() const;
  string DataWidthSpec() const;

private:
  static string WidthSpec(int w);
  void WriteInternal(ostream &os);

  const Module &mod_;
  const IResource &res_;
  bool reset_polarity_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_internal_sram_h_
