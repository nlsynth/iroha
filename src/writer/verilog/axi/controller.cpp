#include "writer/verilog/axi/controller.h"

#include "iroha/i_design.h"
#include "writer/verilog/axi/axi_port.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

Controller::Controller(const IResource &res, bool reset_polarity)
  : res_(res), reset_polarity_(reset_polarity) {
  ports_.reset(new Ports);
  AxiPort::GetReadWrite(res_, &r_, &w_);
  const IResource *mem_res = res_.GetSharedRegister();
  IArray *array = mem_res->GetArray();
  addr_width_ = array->GetAddressWidth();
  data_width_ = array->GetDataType().GetWidth();
}

Controller::~Controller() {
}

void Controller::Write(ostream &os) {
  string name = AxiPort::ControllerName(res_, reset_polarity_);
  ports_->AddPort("clk", Port::INPUT_CLK, 0);
  ports_->AddPort(ResetName(reset_polarity_), Port::INPUT_RESET, 0);
  ports_->AddPort("sram_addr", Port::OUTPUT, addr_width_);
  ports_->AddPort("sram_wdata", Port::OUTPUT, data_width_);
  ports_->AddPort("sram_wen", Port::OUTPUT, 0);
  ports_->AddPort("sram_rdata", Port::INPUT, data_width_);
  ports_->AddPort("addr", Port::INPUT, 32);
  ports_->AddPort("wen", Port::INPUT, 0);
  ports_->AddPort("req", Port::INPUT, 0);
  ports_->AddPort("ack", Port::OUTPUT, 0);
  string initials;
  if (r_) {
    GenReadChannel(nullptr, ports_.get(), &initials);
  }
  if (w_) {
    GenWriteChannel(nullptr, ports_.get(), &initials);
  }
  os << "module " << name << "(";
  ports_->Output(Ports::PORT_NAME, os);
  os << ");\n";
  ports_->Output(Ports::PORT_TYPE, os);
  os << "\n"
     << "  `define S_IDLE 0\n"
     << "  `define S_ADDR_WAIT 1\n";
  if (r_) {
    os << "  `define S_READ_DATA 2\n";
  }
  os << "  reg [3:0] st;\n\n"
     << "  reg [" << (addr_width_ - 1) << ":0] idx;\n\n";
  os << "  always @(posedge clk) begin\n"
     << "    if (" << (reset_polarity_ ? "" : "!")
     << ResetName(reset_polarity_) << ") begin\n"
     << "      ack <= 0;\n"
     << "      sram_wen <= 0;\n"
     << "      st <= `S_IDLE;\n";
  int alen = (1 << addr_width_) - 1;
  if (r_) {
    os << "      ARLEN <= " << alen << ";\n";
    os << "      ARSIZE <= 0;\n";
  }
  os << initials
     << "    end else begin\n";
  OutputFsm(os);
  os << "    end\n"
     << "  end\n"
     << "endmodule\n";
}

string Controller::ResetName(bool polarity) {
  if (polarity) {
    return "rst";
  } else {
    return "rst_n";
  }
}

void Controller::AddPorts(Module *mod, bool r, bool w,
			  string *s) {
  Ports *ports = mod->GetPorts();
  if (r) {
    GenReadChannel(mod, ports, s);
  }
  if (w) {
    GenWriteChannel(mod, ports, s);
  }
}

void Controller::GenReadChannel(Module *module, Ports *ports,
				string *s) {
  // TODO: More ports.
  AddPort("ARADDR", 32, false, module, ports, s);
  AddPort("ARVALID", 0, false, module, ports, s);
  AddPort("ARREADY", 0, true, module, ports, s);
  AddPort("ARLEN", 8, false, module, ports, s);
  AddPort("ARSIZE", 3, false, module, ports, s);

  AddPort("RVALID", 0, true, module, ports, s);
  AddPort("RDATA", 32, true, module, ports, s);
  AddPort("RREADY", 0, false, module, ports, s);
  AddPort("RLAST", 0, true, module, ports, s);
}

void Controller::GenWriteChannel(Module *module, Ports *ports,
				 string *s) {
  AddPort("AWVALID", 0, false, module, ports, s);
  AddPort("AWREADY", 0, true, module, ports, s);
}

void Controller::AddPort(const string &name, int width, bool dir,
			 Module *module, Ports *ports,
			 string *s) {
  Port::PortType t;
  if (dir) {
    t = Port::INPUT;
  } else {
    if (module == nullptr) {
      t = Port::OUTPUT;
    } else {
      t = Port::OUTPUT_WIRE;
    }
  }
  ports->AddPort(name, t, width);
  string p = ", ." + name + "(" + name + ")";
  if (s != nullptr && module != nullptr) {
    *s += p;
  }
  if (module != nullptr) {
    Module *parent = module->GetParentModule();
    if (parent != nullptr) {
      ostream &os = parent->ChildModuleInstSectionStream(module);
      os << p;
    }
  }
  if (module == nullptr) {
    if (!dir) {
      *s = "      " + name + " <= 0;\n";
    }
  }
}

void Controller::OutputFsm(ostream &os) {
  os << "      sram_wen <= (st == `S_READ_DATA && RVALID);\n";
  os << "      case (st)\n"
     << "        `S_IDLE: begin\n"
     << "          if (req) begin\n"
     << "            idx <= 0;\n"
     << "            st <= `S_ADDR_WAIT;\n";
  if (r_ && !w_) {
    os << "            ARVALID <= 1;\n"
       << "            ARADDR <= addr;\n";
  }
  if (!r_ && w_) {
    os << "            AWVALID <= 1;\n";
  }
  if (r_ && w_) {
    os << "            if (wen) begin\n"
       << "              ARVALID <= 1;\n"
       << "              ARADDR <= addr;\n"
       << "            end else begin\n"
       << "              AWVALID <= 1;\n"
       << "            end\n";
  }
  os << "          end\n"
     << "        end\n"
     << "        `S_ADDR_WAIT: begin\n";
  if (r_ && !w_) {
    os << "          if (ARREADY) begin\n"
       << "            st <= `S_READ_DATA;\n"
       << "            ARVALID <= 0;\n"
       << "            RREADY <= 1;\n"
       << "          end\n";
  }
  if (!r_ && w_) {
    os << "            if (AWREADY) begin\n"
       << "            end\n";
  }
  if (r_ && w_) {
    os << "            if (wen) begin\n"
       << "              if (AWREADY) begin\n"
       << "              end\n"
       << "            end else begin\n"
       << "              if (ARREADY) begin\n"
       << "                st <= `S_READ_DATA;\n"
       << "                RREADY <= 1;\n"
       << "              end\n"
       << "            end\n";
  }
  os << "        end\n";
  if (r_) {
    ReaderFsm(os);
  }
  if (w_) {
    WriterFsm(os);
  }
  os << "      endcase\n";
}

void Controller::ReaderFsm(ostream &os) {
  os << "        `S_READ_DATA: begin\n"
     << "          if (RVALID) begin\n"
     << "            sram_addr <= idx;\n"
     << "            sram_wdata <= RDATA;\n"
     << "            idx <= idx + 1;\n"
     << "            if (RLAST) begin\n"
     << "              RREADY <= 0;\n"
     << "              st <= `S_IDLE;\n"
     << "            end\n"
     << "          end\n"
     << "        end\n";
}

void Controller::WriterFsm(ostream &os) {
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
