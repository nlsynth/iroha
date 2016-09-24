#include "writer/verilog/internal_sram.h"

#include "iroha/i_design.h"
#include "writer/verilog/module.h"

namespace iroha {
namespace writer {
namespace verilog {

InternalSRAM::InternalSRAM(const Module &mod, const IResource &res)
  : mod_(mod), res_(res) {
  reset_polarity_ = mod_.GetResetPolarity();
}

void InternalSRAM::Write(ostream &os) {
  os << "// SRAM\n"
     << "module " << GetModuleName() << "(clk, " << GetResetPinName()
     << ", addr_i, rdata_o, wdata_i, write_en_i);\n"
     << "  input clk;\n"
     << "  input " << GetResetPinName() << ";\n"
     << "  input " << AddressWidthSpec() << "addr_i;\n"
     << "  output " << DataWidthSpec() << "rdata_o;\n"
     << "  input " << DataWidthSpec() << "wdata_i;\n"
     << "  input write_en_i;\n\n"
     << "  reg " << DataWidthSpec() << "rdata_o;\n\n";
  WriteInternal(os);
  os << "endmodule\n\n";
}

void InternalSRAM::WriteInternal(ostream &os) {
  IArray *array = res_.GetArray();
  int array_size = 1 << array->GetAddressWidth();
  os << "  reg " << DataWidthSpec()
     << "data [0:" << (array_size - 1) << "];\n\n";
  os << "  always @(posedge clk) begin\n"
     << "    if (";
  if (!reset_polarity_) {
    os << "!";
  }
  os << GetResetPinName() << ") begin\n";
  IArrayImage *im = array->GetArrayImage();
  if (im != nullptr) {
    for (int i = 0; i < im->values_.size(); ++i) {
      os << "      data[" << i << "] <= " << im->values_[i] << ";\n";
    }
  }
  os << "    end else begin\n"
     << "      if (write_en_i) begin\n"
     << "        data[addr_i] <= wdata_i;\n"
     << "      end\n"
     << "    end\n"
     << "  end\n";
  os << "  // Read\n"
     << "  always @(addr_i or clk) begin\n"
     << "    rdata_o = data[addr_i];\n"
     << "  end\n";
}

const IResource &InternalSRAM::GetResource() const {
  return res_;
}

string InternalSRAM::GetModuleName() const {
  IArray *array = res_.GetArray();
  const IValueType &type = array->GetDataType();
  return "SRAM_" + Util::Itoa(array->GetAddressWidth())
    + "_" + Util::Itoa(type.GetWidth());
}

string InternalSRAM::GetResetPinName() const {
  if (reset_polarity_) {
    return "rst";
  } else {
    return "rst_n";
  }
}

string InternalSRAM::AddressWidthSpec() const {
  IArray *array = res_.GetArray();
  return WidthSpec(array->GetAddressWidth());
}

string InternalSRAM::DataWidthSpec() const {
  IArray *array = res_.GetArray();
  return WidthSpec(array->GetDataType().GetWidth());
}

string InternalSRAM::WidthSpec(int w) {
  if (w == 0) {
    return string();
  }
  return "[" + Util::Itoa(w - 1) + ":0] ";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
