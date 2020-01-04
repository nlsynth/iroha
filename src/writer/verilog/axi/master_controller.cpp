#include "writer/verilog/axi/master_controller.h"

#include "iroha/i_design.h"
#include "writer/verilog/axi/channel_generator.h"
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
  ports_->AddPort("len", Port::INPUT, cfg_.sram_addr_width);
  ports_->AddPort("start", Port::INPUT, cfg_.sram_addr_width);
  ports_->AddPort("wen", Port::INPUT, 0);
  ports_->AddPort("req", Port::INPUT, 0);
  ports_->AddPort("ack", Port::OUTPUT, 0);
  string initials;
  ChannelGenerator ch(cfg_, ChannelGenerator::CONTROLLER_PORTS_AND_REG_INITIALS,
		      true, nullptr, ports_.get(), &initials);
  ch.GenerateChannel(true, true);
  string name = MasterPort::ControllerName(res_);
  WriteModuleHeader(name, os);
  ports_->Output(Ports::PORT_MODULE_HEAD, os);
  os << ");\n";
  ports_->Output(Ports::FIXED_VALUE_ASSIGN, os);
  os << "\n"
     << "  localparam S_IDLE = 0;\n"
     << "  localparam S_ADDR_WAIT = 1;\n";
  if (r_) {
    os << "  localparam S_READ_DATA = 2;\n"
       << "  localparam S_READ_DATA_WAIT = 3;\n";
  }
  if (w_) {
    os << "  localparam S_WRITE_WAIT = 4;\n";
  }
  os << "  reg [2:0] st;\n\n";
  os << "  reg [" << (cfg_.sram_addr_width -  1) << ":0] sram_addr_src;\n";
  os << "  wire [" << (cfg_.sram_addr_width -  1) << ":0] next_sram_addr;\n";
  if (w_) {
    // Outputs next address immediately from the combinational logic,
    // if handshake happens.
    os << "  assign sram_addr = (WREADY && WVALID && sram_EXCLUSIVE) ? next_sram_addr : sram_addr_src;\n";
  } else {
    os << "  assign sram_addr = sram_addr_src;\n";
  }
  os << "  assign next_sram_addr = sram_addr_src + 1;\n\n";
  if (w_) {
    os << "  localparam WS_IDLE = 0;\n"
       << "  localparam WS_WRITE = 1;\n"
       << "  localparam WS_WAIT = 2;\n"
       << "  localparam WS_SRAM = 4;\n"
       << "  localparam WS_AXI = 5;\n"
       << "  reg [2:0] wst;\n"
       << "  reg [" << cfg_.sram_addr_width << ":0] wmax;\n\n";
  }
  if (r_) {
    os << "  reg [" << cfg_.sram_addr_width << ":0] ridx;\n"
       << "  reg read_last;\n\n";
  }
  if (w_) {
    os << "  reg [" << cfg_.sram_addr_width << ":0] widx;\n\n";
  }
  os << "  always @(posedge clk) begin\n"
     << "    if (" << (reset_polarity_ ? "" : "!")
     << ResetName(reset_polarity_) << ") begin\n"
     << "      ack <= 0;\n"
     << "      sram_req <= 0;\n"
     << "      sram_wen <= 0;\n"
     << "      st <= S_IDLE;\n";
  if (w_) {
    os << "      wst <= WS_IDLE;\n"
       << "      wmax <= 0;\n";
  }
  os << initials
     << "    end else begin\n";
  OutputMainFsm(os);
  if (w_) {
    OutputWriterFsm(os);
  }
  os << "    end\n"
     << "  end\n";
  WriteModuleFooter(name, os);
}

void MasterController::AddPorts(const PortConfig &cfg,
				Module *mod, bool r, bool w,
				string *s) {
  Ports *ports = mod->GetPorts();
  ChannelGenerator ch(cfg, ChannelGenerator::PORTS_TO_EXT_AND_CONNECTIONS,
		      true, mod, ports, s);
  ch.GenerateChannel(true, true);
}

void MasterController::OutputMainFsm(ostream &os) {
  if (r_) {
    os << "      if (sram_EXCLUSIVE) begin\n"
       << "        sram_wen <= (st == S_READ_DATA && RVALID);\n"
       << "      end\n";
  } else {
    os << "      sram_wen <= 0;\n";
  }
  int rwsize;
  if (cfg_.data_width == 32) {
    rwsize = 2;
  } else {
    // 64bit.
    rwsize = 3;
  }
  os << "      case (st)\n"
     << "        S_IDLE: begin\n"
     << "          ack <= 0;\n";
  if (r_ || w_) {
    os << "          if (req && !ack) begin\n";
    if (r_) {
      os << "            ridx <= 0;\n";
    }
    os << "            st <= S_ADDR_WAIT;\n";
    if (r_ && !w_) {
      os << "            ARVALID <= 1;\n"
	 << "            ARADDR <= addr;\n"
	 << "            ARLEN <= len;\n"
	 << "            ARSIZE <= " << rwsize << ";\n";
    }
    if (!r_ && w_) {
      os << "            AWVALID <= 1;\n"
	 << "            AWADDR <= addr;\n"
	 << "            AWLEN <= len;\n"
	 << "            AWSIZE <= " << rwsize << ";\n"
	 << "            sram_addr_src <= start;\n"
	 << "            wmax <= len;\n";
    }
    if (r_ && w_) {
      os << "            if (wen) begin\n"
	 << "              AWVALID <= 1;\n"
	 << "              AWADDR <= addr;\n"
	 << "              AWLEN <= len;\n"
	 << "              AWSIZE <= " << rwsize << ";\n"
	 << "              sram_addr_src <= start;\n"
	 << "              wmax <= len;\n"
	 << "            end else begin\n"
	 << "              ARVALID <= 1;\n"
	 << "              ARADDR <= addr;\n"
	 << "              ARLEN <= len;\n"
	 << "              ARSIZE <= " << rwsize << ";\n"
	 << "            end\n";
    }
    os << "          end\n";
  }
  os << "        end\n";
  if (r_ || w_) {
    os << "        S_ADDR_WAIT: begin\n"
       << "          ack <= 0;\n";
    if (r_ && !w_) {
      os << "          if (ARREADY) begin\n"
	 << "            st <= S_READ_DATA;\n"
	 << "            ARVALID <= 0;\n"
	 << "            RREADY <= 1;\n"
	 << "          end\n";
    }
    if (!r_ && w_) {
      os << "          if (AWREADY) begin\n"
	 << "            st <= S_WRITE_WAIT;\n"
	 << "            AWVALID <= 0;\n"
	 << "            sram_addr_src <= start;\n"
	 << "            if (!sram_EXCLUSIVE) begin\n"
	 << "              sram_req <= 1;\n"
	 << "            end\n"
	 << "          end\n";
    }
    if (r_ && w_) {
      os << "          if (AWVALID) begin\n"
	 << "            if (AWREADY) begin\n"
	 << "              st <= S_WRITE_WAIT;\n"
	 << "              AWVALID <= 0;\n"
	 << "              if (!sram_EXCLUSIVE) begin\n"
	 << "                sram_req <= 1;\n"
	 << "              end\n"
	 << "            end\n"
	 << "          end else begin\n"
	 << "            if (ARREADY) begin\n"
	 << "              st <= S_READ_DATA;\n"
	 << "              ARVALID <= 0;\n"
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
    os << "        S_WRITE_WAIT: begin\n"
       << "          if (BVALID) begin\n"
       << "            ack <= 1;\n"
       << "            st <= S_IDLE;\n"
       << "          end\n"
       << "          if (wst == WS_WRITE) begin\n"  // sram_EXCLUSIVE
       << "            if (widx == 0 || (WREADY && WVALID)) begin\n"
       << "              sram_addr_src <= next_sram_addr;\n"
       << "            end\n"
       << "          end\n"
       << "          if (wst == WS_SRAM) begin\n"  // !sram_EXCLUSIVE
       << "          if (sram_ack) begin\n"
       << "              sram_req <= 0;\n"
       << "            end\n"
       << "          end\n"
       << "          if (wst == WS_AXI) begin\n"  // !sram_EXCLUSIVE
       << "            if (WREADY && WVALID) begin\n"
       << "              if (widx <= wmax) begin\n"
       << "                sram_req <= 1;\n"
       << "                sram_addr_src <= next_sram_addr;\n"
       << "              end\n"
       << "            end\n"
       << "          end\n"
       << "        end\n";
  }
  os << "      endcase\n";
}

void MasterController::ReadState(ostream &os) {
  os << "        S_READ_DATA: begin\n"
     << "          if (RVALID) begin\n"
     << "            sram_addr_src <= start + ridx;\n"
     << "            sram_wdata <= RDATA;\n"
     << "            ridx <= ridx + 1;\n"
     << "            if (sram_EXCLUSIVE) begin\n"
     << "              if (RLAST) begin\n"
     << "                RREADY <= 0;\n"
     << "                ack <= 1;\n"
     << "                st <= S_IDLE;\n"
     << "              end\n"
     << "            end else begin\n"
     << "              st <= S_READ_DATA_WAIT;\n"
     << "              sram_req <= 1;\n"
     << "              sram_wen <= 1;\n"
     << "              RREADY <= 0;\n"
     << "              read_last <= RLAST;\n"
     << "            end\n"
     << "          end\n"
     << "        end\n"
     << "        S_READ_DATA_WAIT: begin\n" // !sram_EXCLUSIVE
     << "          if (sram_ack) begin\n"
     << "            sram_req <= 0;\n"
     << "            sram_wen <= 0;\n"
     << "            if (read_last) begin\n"
     << "              ack <= 1;\n"
     << "              st <= S_IDLE;\n"
     << "            end else begin\n"
     << "              st <= S_READ_DATA;\n"
     << "              RREADY <= 1;\n"
     << "            end\n"
     << "          end\n"
     << "        end\n";
}

void MasterController::OutputWriterFsm(ostream &os) {
  os << "      case (wst)\n"
     << "        WS_IDLE: begin\n"
     << "          if (AWVALID) begin\n"
     << "            if (sram_EXCLUSIVE) begin\n"
     << "              wst <= WS_WRITE;\n"
     << "            end else begin\n"
     << "              wst <= WS_SRAM;\n"
     << "            end\n"
     << "            widx <= 0;\n"
     << "          end\n"
     << "        end\n"
     << "        WS_WRITE: begin\n" // sram_EXCLUSIVE
     << "          if (widx <= wmax) begin\n"
     << "            WVALID <= 1;\n"
     << "            WDATA <= sram_rdata;\n"
     << "            if (widx == wmax) begin\n"
     << "              WLAST <= 1;\n"
     << "            end\n"
     << "            if (widx == 0 || (WREADY && WVALID)) begin\n"
     << "              widx <= widx + 1;\n"
     << "            end\n"
     << "          end else begin\n"
     << "            WVALID <= 0;\n"
     << "            WLAST <= 0;\n"
     << "            wst <= WS_WAIT;\n"
     << "            BREADY <= 1;\n"
     << "          end\n"
     << "        end\n"
     << "        WS_SRAM: begin\n" // !sram_EXCLUSIVE
     << "          if (sram_ack) begin\n"
     << "            wst <= WS_AXI;\n"
     << "            WDATA <= sram_rdata;\n"
     << "            widx <= widx + 1;\n"
     << "            WVALID <= 1;\n"
     << "            if (widx == wmax) begin\n"
     << "              WLAST <= 1;\n"
     << "            end\n"
     << "          end\n"
     << "        end\n"
     << "        WS_AXI: begin\n" // !sram_EXCLUSIVE
     << "          if (WREADY && WVALID) begin\n"
     << "            WVALID <= 0;\n"
     << "            if (widx <= wmax) begin\n"
     << "              wst <= WS_SRAM;\n"
     << "            end else begin\n"
     << "              WLAST <= 0;\n"
     << "              wst <= WS_WAIT;\n"
     << "            end\n"
     << "          end\n"
     << "        end\n"
     << "        WS_WAIT: begin\n"
     << "          if (BVALID) begin\n"
     << "            BREADY <= 0;\n"
     << "            wst <= WS_IDLE;\n"
     << "          end\n"
     << "        end\n"
     << "      endcase\n";

}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
