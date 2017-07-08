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
  : AxiController(res, reset_polarity) {
  MasterPort::GetReadWrite(res_, &r_, &w_);
}

MasterController::~MasterController() {
}

void MasterController::Write(ostream &os) {
  AddSramPorts();
  ports_->AddPort("addr", Port::INPUT, 32);
  ports_->AddPort("len", Port::INPUT, sram_addr_width_);
  ports_->AddPort("start", Port::INPUT, sram_addr_width_);
  ports_->AddPort("wen", Port::INPUT, 0);
  ports_->AddPort("req", Port::INPUT, 0);
  ports_->AddPort("ack", Port::OUTPUT, 0);
  string initials;
  GenReadChannel(cfg_, true, nullptr, ports_.get(), &initials);
  GenWriteChannel(cfg_, true, nullptr, ports_.get(), &initials);
  string name = MasterPort::ControllerName(res_, reset_polarity_);
  os << "module " << name << "(";
  ports_->Output(Ports::PORT_NAME, os);
  os << ");\n";
  ports_->Output(Ports::PORT_TYPE, os);
  os << "\n"
     << "  `define S_IDLE 0\n"
     << "  `define S_ADDR_WAIT 1\n";
  if (r_) {
    os << "  `define S_READ_DATA 2\n"
       << "  `define S_READ_DATA_WAIT 3\n";
  }
  if (w_) {
    os << "  `define S_WRITE_WAIT 4\n";
  }
  os << "  reg [2:0] st;\n\n";
  if (w_) {
    os << "  `define WS_IDLE 0\n"
       << "  `define WS_WRITE 1\n"
       << "  `define WS_WAIT 2\n"
       << "  reg [1:0] wst;\n"
       << "  reg [" << sram_addr_width_ << ":0] wmax;\n\n";
  }
  if (r_) {
    os << "  reg [" << sram_addr_width_ << ":0] ridx;\n"
       << "  reg read_last;\n\n";
  }
  if (r_) {
    os << "  reg [" << sram_addr_width_ << ":0] widx;\n\n";
  }
  os << "  always @(posedge clk) begin\n"
     << "    if (" << (reset_polarity_ ? "" : "!")
     << ResetName(reset_polarity_) << ") begin\n"
     << "      ack <= 0;\n"
     << "      sram_req <= 0;\n"
     << "      sram_wen <= 0;\n"
     << "      st <= `S_IDLE;\n";
  if (w_) {
    os << "      wst <= `WS_IDLE;\n"
       << "      wmax <= 0;\n";
  }
  os << initials
     << "    end else begin\n";
  OutputMainFsm(os);
  if (w_) {
    OutputWriterFsm(os);
  }
  os << "    end\n"
     << "  end\n"
     << "endmodule\n";
}

void MasterController::AddPorts(const PortConfig &cfg,
				Module *mod, bool r, bool w,
				string *s) {
  Ports *ports = mod->GetPorts();
  if (r) {
    GenReadChannel(cfg, true, mod, ports, s);
  }
  if (w) {
    GenWriteChannel(cfg, true, mod, ports, s);
  }
}

void MasterController::OutputMainFsm(ostream &os) {
  if (r_) {
    os << "      if (sram_EXCLUSIVE) begin\n"
       << "        sram_wen <= (st == `S_READ_DATA && RVALID);\n"
       << "      end\n";
  } else {
    os << "      sram_wen <= 0;\n";
  }
  os << "      case (st)\n"
     << "        `S_IDLE: begin\n";
  if (r_ || w_) {
    os << "          if (req) begin\n";
    if (r_) {
      os << "            ridx <= 0;\n";
    }
    os << "            st <= `S_ADDR_WAIT;\n";
    if (r_ && !w_) {
      os << "            ARVALID <= 1;\n"
	 << "            ARADDR <= addr;\n"
	 << "            ARLEN <= len;\n";
    }
    if (!r_ && w_) {
      os << "            AWVALID <= 1;\n"
	 << "            AWADDR <= addr;\n"
	 << "            AWLEN <= len;\n";
    }
    if (r_ && w_) {
      os << "            if (wen) begin\n"
	 << "              ARVALID <= 1;\n"
	 << "              ARADDR <= addr;\n"
	 << "              ARLEN <= len;\n"
	 << "            end else begin\n"
	 << "              AWVALID <= 1;\n"
	 << "              AWADDR <= addr;\n"
	 << "              AWLEN <= len;\n"
	 << "              wmax <= len;\n"
	 << "            end\n";
    }
    os << "          end\n";
  }
  os << "        end\n";
  if (r_ || w_) {
    os << "        `S_ADDR_WAIT: begin\n";
    if (r_ && !w_) {
      os << "          if (ARREADY) begin\n"
	 << "            st <= `S_READ_DATA;\n"
	 << "            ARVALID <= 0;\n"
	 << "            RREADY <= 1;\n"
	 << "          end\n";
    }
    if (!r_ && w_) {
      os << "          if (AWREADY) begin\n"
	 << "            st <= `S_WRITE_WAIT;\n"
	 << "            AWVALID <= 0;\n"
	 << "          end\n";
    }
    if (r_ && w_) {
      os << "          if (wen) begin\n"
	 << "            if (AWREADY) begin\n"
	 << "              st <= `S_WRITE_WAIT;\n"
	 << "              AWVALID <= 0;\n"
	 << "              sram_addr <= ridx;\n"
	 << "            end\n"
	 << "          end else begin\n"
	 << "            if (ARREADY) begin\n"
	 << "              st <= `S_READ_DATA;\n"
	 << "              RREADY <= 1;\n"
	 << "            end\n"
	 << "          end\n";
    }
    os << "        end\n";
  }
  if (r_) {
    ReadState(os);
  }
  if (w_) {
    os << "        `S_WRITE_WAIT: begin\n"
       << "          if (BVALID) begin\n"
       << "            st <= `S_IDLE;\n"
       << "          end\n"
       << "          if (wst == `WS_IDLE && req && wen) begin\n"
       << "            sram_addr <= start;\n"
       << "            if (!sram_EXCLUSIVE) begin\n"
       << "              sram_req <= 1;\n"
       << "            end\n"
       << "          end\n"
       << "          if (wst == `WS_WRITE) begin\n"
       << "            if (WREADY && WVALID) begin\n"
       << "              sram_addr <= sram_addr + 1;\n"
       << "              if (!sram_EXCLUSIVE) begin\n"
       << "                sram_req <= 1;\n"
       << "              end\n"
       << "            end else if (sram_ack && !sram_EXCLUSIVE) begin\n"
       << "              sram_req <= 0;\n"
       << "            end\n"
       << "          end\n"
       << "        end\n";
  }
  os << "      endcase\n";
}

void MasterController::ReadState(ostream &os) {
  os << "        `S_READ_DATA: begin\n"
     << "          if (RVALID) begin\n"
     << "            sram_addr <= start + ridx;\n"
     << "            sram_wdata <= RDATA;\n"
     << "            ridx <= ridx + 1;\n"
     << "            if (sram_EXCLUSIVE) begin\n"
     << "              if (RLAST) begin\n"
     << "                RREADY <= 0;\n"
     << "                st <= `S_IDLE;\n"
     << "              end\n"
     << "            end else begin\n"
     << "              st <= `S_READ_DATA_WAIT;\n"
     << "              sram_req <= 1;\n"
     << "              sram_wen <= 1;\n"
     << "              RREADY <= 0;\n"
     << "              read_last <= RLAST;\n"
     << "            end\n"
     << "          end\n"
     << "        end\n"
     << "        `S_READ_DATA_WAIT: begin\n" // !sram_EXCLUSIVE
     << "          if (sram_ack) begin\n"
     << "            sram_req <= 0;\n"
     << "            sram_wen <= 0;\n"
     << "            if (read_last) begin\n"
     << "              st <= `S_IDLE;\n"
     << "            end else begin\n"
     << "              st <= `S_READ_DATA;\n"
     << "              RREADY <= 1;\n"
     << "            end\n"
     << "          end\n"
     << "        end\n";
}

void MasterController::OutputWriterFsm(ostream &os) {
  os << "      case (wst)\n"
     << "        `WS_IDLE: begin\n"
     << "          if (req && wen) begin\n"
     << "            wst <= `WS_WRITE;\n"
     << "            widx <= 0;\n"
     << "          end\n"
     << "        end\n"
     << "        `WS_WRITE: begin\n"
     << "          if (widx <= wmax) begin\n"
     << "            if (sram_EXCLUSIVE || sram_ack) begin\n"
     << "              WVALID <= 1;\n"
     << "              WDATA <= sram_rdata;\n"
     << "              if (widx == wmax) begin\n"
     << "                WLAST <= 1;\n"
     << "              end\n"
     << "            end\n"
     << "            if (WREADY && WVALID) begin\n"
     << "              widx <= widx + 1;\n"
     << "              if (!sram_EXCLUSIVE) begin\n"
     << "                WVALID <= 0;\n"
     << "              end\n"
     << "            end\n"
     << "          end else begin\n"
     << "            WVALID <= 0;\n"
     << "            WLAST <= 0;\n"
     << "            wst <= `WS_WAIT;\n"
     << "            BREADY <= 1;\n"
     << "          end\n"
     << "        end\n"
     << "        `WS_WAIT: begin\n"
     << "          if (BVALID) begin\n"
     << "            BREADY <= 0;\n"
     << "            st <= `WS_IDLE;\n"
     << "          end\n"
     << "        end\n"
     << "      endcase\n";

}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
