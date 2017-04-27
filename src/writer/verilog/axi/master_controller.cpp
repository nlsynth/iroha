#include "writer/verilog/axi/master_controller.h"

#include "iroha/i_design.h"
#include "writer/verilog/axi/master_port.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"
#include "writer/verilog/table.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

MasterController::MasterController(const IResource &res, bool reset_polarity)
  : res_(res), reset_polarity_(reset_polarity) {
  ports_.reset(new Ports);
  MasterPort::GetReadWrite(res_, &r_, &w_);
  const IResource *mem_res = res_.GetParentResource();
  IArray *array = mem_res->GetArray();
  addr_width_ = array->GetAddressWidth();
  data_width_ = array->GetDataType().GetWidth();
  burst_len_ = (1 << addr_width_);
}

MasterController::~MasterController() {
}

void MasterController::Write(ostream &os) {
  string name = MasterPort::ControllerName(res_, reset_polarity_);
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
  if (w_) {
    os << "  `define S_WRITE_DATA 3\n";
    os << "  `define S_WRITE_WAIT 4\n";
  }
  os << "  reg [3:0] st;\n\n"
     << "  reg [" << addr_width_ << ":0] idx;\n\n";
  os << "  always @(posedge clk) begin\n"
     << "    if (" << (reset_polarity_ ? "" : "!")
     << ResetName(reset_polarity_) << ") begin\n"
     << "      ack <= 0;\n"
     << "      sram_wen <= 0;\n"
     << "      st <= `S_IDLE;\n";
  os << initials
     << "    end else begin\n";
  OutputFsm(os);
  os << "    end\n"
     << "  end\n"
     << "endmodule\n";
}

string MasterController::ResetName(bool polarity) {
  if (polarity) {
    return "rst";
  } else {
    return "rst_n";
  }
}

void MasterController::AddPorts(Module *mod, bool r, bool w,
				string *s) {
  Ports *ports = mod->GetPorts();
  if (r) {
    GenReadChannel(mod, ports, s);
  }
  if (w) {
    GenWriteChannel(mod, ports, s);
  }
}

void MasterController::GenReadChannel(Module *module, Ports *ports,
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

void MasterController::GenWriteChannel(Module *module, Ports *ports,
				       string *s) {
  AddPort("AWADDR", 32, false, module, ports, s);
  AddPort("AWVALID", 0, false, module, ports, s);
  AddPort("AWREADY", 0, true, module, ports, s);
  AddPort("AWLEN", 8, false, module, ports, s);
  AddPort("AWSIZE", 3, false, module, ports, s);

  AddPort("WVALID", 0, false, module, ports, s);
  AddPort("WREADY", 0, true, module, ports, s);
  AddPort("WDATA", 32, false, module, ports, s);
  AddPort("WLAST", 0, false, module, ports, s);

  AddPort("BVALID", 0, true, module, ports, s);
  AddPort("BREADY", 0, false, module, ports, s);
  AddPort("BRESP", 2, true, module, ports, s);
}

void MasterController::AddPort(const string &name, int width, bool dir,
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
      *s += "      " + name + " <= 0;\n";
    }
  }
}

void MasterController::OutputFsm(ostream &os) {
  int alen = burst_len_ - 1;
  if (r_) {
    os << "      sram_wen <= (st == `S_READ_DATA && RVALID);\n";
  } else {
    os << "      sram_wen <= 0;\n";
  }
  os << "      case (st)\n"
     << "        `S_IDLE: begin\n"
     << "          if (req) begin\n"
     << "            idx <= 0;\n"
     << "            st <= `S_ADDR_WAIT;\n";
  if (r_ && !w_) {
    os << "            ARVALID <= 1;\n"
       << "            ARADDR <= addr;\n"
       << "            ARLEN <= " << alen << ";\n";
  }
  if (!r_ && w_) {
    os << "            AWVALID <= 1;\n"
       << "            AWADDR <= addr;\n"
       << "            AWLEN <= " << alen << ";\n";
  }
  if (r_ && w_) {
    os << "            if (wen) begin\n"
       << "              ARVALID <= 1;\n"
       << "              ARADDR <= addr;\n"
       << "              ARLEN <= " << alen << ";\n"
       << "            end else begin\n"
       << "              AWVALID <= 1;\n"
       << "              AWADDR <= addr;\n"
       << "              AWLEN <= " << alen << ";\n"
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
    os << "          if (AWREADY) begin\n"
       << "            st <= `S_WRITE_DATA;\n"
       << "            AWVALID <= 0;\n"
       << "            sram_addr <= idx;\n"
       << "          end\n";
  }
  if (r_ && w_) {
    os << "          if (wen) begin\n"
       << "            if (AWREADY) begin\n"
       << "              st <= `S_WRITE_DATA;\n"
       << "              AWVALID <= 0;\n"
       << "              sram_addr <= idx;\n"
       << "            end\n"
       << "          end else begin\n"
       << "            if (ARREADY) begin\n"
       << "              st <= `S_READ_DATA;\n"
       << "              RREADY <= 1;\n"
       << "            end\n"
       << "          end\n";
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

void MasterController::ReaderFsm(ostream &os) {
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

void MasterController::WriterFsm(ostream &os) {
  os << "        `S_WRITE_DATA: begin\n"
     << "          if (idx < " << burst_len_ << ") begin\n"
     << "            WVALID <= 1;\n"
     << "            WDATA <= sram_rdata;\n"
     << "            if (WREADY && WVALID) begin\n"
     << "              sram_addr <= idx + 1;\n"
     << "              idx <= idx + 1;\n"
     << "            end\n"
     << "            if (idx < " << burst_len_ << " - 1) begin\n"
     << "              WLAST <= 0;\n"
     << "            end\n"
     << "          end else begin\n"
     << "            WVALID <= 0;\n"
     << "            WLAST <= 0;\n"
     << "            st <= `S_WRITE_WAIT;\n"
     << "            BREADY <= 1;\n"
     << "          end\n"
     << "        end\n"
     << "        `S_WRITE_WAIT: begin\n"
     << "          if (BVALID) begin\n"
     << "            BREADY <= 0;\n"
     << "            st <= `S_IDLE;\n"
     << "          end\n"
     << "        end\n";
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
