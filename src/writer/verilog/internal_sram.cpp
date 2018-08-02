#include "writer/verilog/internal_sram.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "writer/verilog/module.h"

namespace iroha {
namespace writer {
namespace verilog {

InternalSRAM::InternalSRAM(const Module &mod, const IArray &array,
			   int num_ports)
  : mod_(mod), num_ports_(num_ports) {
  CHECK(num_ports == 1 || num_ports == 2);
  reset_polarity_ = mod_.GetResetPolarity();
  // Take a copy.
  array_.reset(new IArray(array));
}

InternalSRAM::~InternalSRAM() {
}

void InternalSRAM::Write(ostream &os) {
  string name = GetModuleName();
  string guard = name + "_defined";
  os << "\n"
     << "// SRAM("<< num_ports_ << " port(s))\n"
     << "`ifndef " << guard << "\n"
     << " `define " << guard << "\n"
     << "module " << name << "(clk, " << GetResetPinName() << ", ";
  for (int p = 0; p < num_ports_; ++p) {
    if (p > 0) {
      os << ", ";
    }
    os << GetAddrPin(p) << ", "
       << GetRdataPin(p) << ", "
       << GetWdataPin(p) << ", "
       << GetWenPin(p);
  }
  os << ");\n"
     << "  input clk;\n"
     << "  input " << GetResetPinName() << ";\n";
  for (int p = 0; p < num_ports_; ++p) {
    os << "  input " << AddressWidthSpec() << GetAddrPin(p) << ";\n"
       << "  output " << DataWidthSpec() << GetRdataPin(p) << ";\n"
       << "  input " << DataWidthSpec() << GetWdataPin(p) << ";\n"
       << "  input " << GetWenPin(p) << ";\n\n"
       << "  reg " << DataWidthSpec() << GetRdataPin(p) << ";\n\n";
  }
  WriteInternal(os);
  os << "endmodule\n"
     << "`endif  // " + guard + "\n"
     << "\n";
}

void InternalSRAM::WriteInternal(ostream &os) {
  int array_size = 1 << array_->GetAddressWidth();
  os << "  reg " << DataWidthSpec()
     << "data [0:" << (array_size - 1) << "];\n\n";
  for (int p = 0; p < num_ports_; ++p) {
    os << "  always @(posedge clk) begin\n"
       << "    if (";
    if (!reset_polarity_) {
      os << "!";
    }
    os << GetResetPinName() << ") begin\n";
    if (p == 0) {
      IArrayImage *im = array_->GetArrayImage();
      if (im != nullptr) {
	for (int i = 0; i < im->values_.size(); ++i) {
	  os << "      data[" << i << "] <= " << im->values_[i] << ";\n";
	}
      }
    }
    os << "    end else begin\n";
    os << "      if (" << GetWenPin(p) << ") begin\n"
       << "        data[" << GetAddrPin(p) << "] <= " << GetWdataPin(p) << ";\n"
       << "      end\n";
    os << "    end\n"
       << "  end\n";
  }
  os << "  // Read\n";
  for (int p = 0; p < num_ports_; ++p) {
    os << "  always @(" << GetAddrPin(p) << " or clk) begin\n"
       << "    " << GetRdataPin(p) << " = data[" << GetAddrPin(p) << "];\n"
       << "  end\n";
  }
}

string InternalSRAM::GetModuleName() const {
  const IValueType &type = array_->GetDataType();
  string n = "SRAM_" + Util::Itoa(array_->GetAddressWidth())
    + "_" + Util::Itoa(type.GetWidth());
  if (num_ports_ == 2) {
    n += "_2";
  }
  return n;
}

string InternalSRAM::GetResetPinName() const {
  if (reset_polarity_) {
    return "rst";
  } else {
    return "rst_n";
  }
}

string InternalSRAM::AddressWidthSpec() const {
  return WidthSpec(array_->GetAddressWidth());
}

string InternalSRAM::DataWidthSpec() const {
  return WidthSpec(array_->GetDataType().GetWidth());
}

string InternalSRAM::WidthSpec(int w) {
  if (w == 0) {
    return string();
  }
  return "[" + Util::Itoa(w - 1) + ":0] ";
}

string InternalSRAM::GetAddrPin(int port) const {
  return "addr_" + MaybePortPrefix(port) +"i";
}

string InternalSRAM::GetRdataPin(int port) const {
  return "rdata_" + MaybePortPrefix(port) + "o";
}

string InternalSRAM::GetWenPin(int port) const {
  return "write_en_" + MaybePortPrefix(port) + "i";
}

string InternalSRAM::GetWdataPin(int port) const {
  return "wdata_" + MaybePortPrefix(port) + "i";
}

string InternalSRAM::MaybePortPrefix(int port) const {
  if (num_ports_ == 2) {
    return Util::Itoa(port) + "_";
  }
  return "";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
