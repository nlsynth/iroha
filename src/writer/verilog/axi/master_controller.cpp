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
  : AxiController(res, reset_polarity_) {
  MasterPort::GetReadWrite(res_, &r_, &w_);
  burst_len_ = (1 << addr_width_);
}

MasterController::~MasterController() {
}

void MasterController::Write(ostream &os) {
  AddSramPorts();
  ports_->AddPort("addr", Port::INPUT, 32);
  ports_->AddPort("wen", Port::INPUT, 0);
  ports_->AddPort("req", Port::INPUT, 0);
  ports_->AddPort("ack", Port::OUTPUT, 0);
  string initials;
  if (r_) {
    GenReadChannel(true, nullptr, ports_.get(), &initials);
  }
  if (w_) {
    GenWriteChannel(true, nullptr, ports_.get(), &initials);
  }
  string name = MasterPort::ControllerName(res_, reset_polarity_);
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

void MasterController::AddPorts(Module *mod, bool r, bool w,
				string *s) {
  Ports *ports = mod->GetPorts();
  if (r) {
    GenReadChannel(true, mod, ports, s);
  }
  if (w) {
    GenWriteChannel(true, mod, ports, s);
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
