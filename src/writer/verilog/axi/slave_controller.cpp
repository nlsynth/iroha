#include "writer/verilog/axi/slave_controller.h"

#include "writer/verilog/axi/channel_generator.h"
#include "writer/verilog/axi/slave_port.h"
#include "writer/verilog/module.h"
#include "writer/verilog/ports.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {  

SlaveController::SlaveController(const IResource &res,
				 bool reset_polarity)
  : AxiController(res, reset_polarity) {
  ports_.reset(new Ports);
}

SlaveController::~SlaveController() {
}

void SlaveController::Write(ostream &os) {
  string name = SlavePort::ControllerName(res_);
  os << "// slave controller: "
     << name << "\n";
  AddSramPorts();
  AddNotifierPorts();
  string initials;
  ChannelGenerator ch(cfg_,
		      ChannelGenerator::CONTROLLER_PORTS_AND_REG_INITIALS,
		      false, nullptr, ports_.get(), &initials);
  ch.GenerateChannel(true, true);
  WriteModuleHeader(name, os);
  ports_->Output(Ports::PORT_MODULE_HEAD, os);
  os << ");\n";
  ports_->Output(Ports::FIXED_VALUE_ASSIGN, os);

  os << "  localparam S_IDLE = 0;\n"
     << "  localparam S_WRITE_DONE = 7;\n"
     << "  // single cycle mode (sram_EXCLUSIVE)\n"
     << "  localparam S_READ = 1;\n"
     << "  localparam S_WRITE = 2;\n"
     << "  // multi cycle mode (!sram_EXCLUSIVE)\n"
     << "  localparam S_READ_AXI = 3;\n"
     << "  localparam S_READ_SRAM = 4;\n"
     << "  localparam S_WRITE_AXI = 5;\n"
     << "  localparam S_WRITE_SRAM = 6;\n";
  os << "  reg [3:0] st;\n\n";
  os << "  reg [" << cfg_.sram_addr_width << ":0] idx;\n\n";
  os << "  reg first_addr;\n"
     << "  reg last_write;\n"
     << "  reg [7:0] rlen;\n\n";
  os << "  reg [" << cfg_.SramMSB() << ":0] sram_addr_src;\n"
     << "  assign sram_addr = sram_addr_src;\n\n";

  os << "  always @(posedge clk) begin\n"
     << "    if (" << (reset_polarity_ ? "" : "!")
     << ResetName(reset_polarity_) << ") begin\n"
     << "      st <= S_IDLE;\n"
     << "      first_addr <= 0;\n"
     << "      last_write <= 0;\n"
     << "      rlen <= 0;\n"
     << "      sram_req <= 0;\n"
     << "      access_notify <= 0;\n";
  os << initials
     << "    end else begin\n";
  OutputFSM(os);
  os << "    end\n"
     << "  end\n";
  WriteModuleFooter(name, os);
}

void SlaveController::AddPorts(const PortConfig &cfg, Module *mod, string *s) {
  Ports *ports = mod->GetPorts();
  ChannelGenerator ch(cfg, ChannelGenerator::PORTS_TO_EXT_AND_CONNECTIONS,
		      false, mod, ports, s);
  ch.GenerateChannel(true, true);
}

void SlaveController::OutputFSM(ostream &os) {
  int data_shift = Util::Log2(cfg_.data_width / 8);
  os << "      sram_wen <= (st == S_WRITE && WVALID);\n"
     << "      case (st)\n"
     << "        S_IDLE: begin\n"
     << "          if (access_ack) begin\n"
     << "            access_notify <= 0;\n"
     << "          end\n"
     << "          if (ARVALID) begin\n"
     << "            if (ARREADY) begin\n"
     << "              ARREADY <= 0;\n"
     << "              sram_addr_src <= ARADDR[" << (cfg_.sram_addr_width - 1 + data_shift) << ":" << data_shift << "];\n"
     << "              rlen <= ARLEN;\n"
     << "              if (sram_EXCLUSIVE) begin\n"
     << "                st <= S_READ;\n"
     << "              end else begin\n"
     << "                st <= S_READ_SRAM;\n"
     << "                sram_req <= 1;\n"
     << "              end\n"
     << "            end else begin\n"
     << "              ARREADY <= 1;\n"
     << "            end\n"
     << "          end else if (AWVALID) begin\n"
     << "            if (AWREADY) begin\n"
     << "              AWREADY <= 0;\n"
     << "              first_addr <= 1;\n"
     << "              sram_addr_src <= AWADDR[" << (cfg_.sram_addr_width - 1 + data_shift) << ":" << data_shift << "];\n"
     << "              if (sram_EXCLUSIVE) begin\n"
     << "                st <= S_WRITE;\n"
     << "              end else begin\n"
     << "                st <= S_WRITE_AXI;\n"
     << "              end\n"
     << "              WREADY <= 1;\n"
     << "            end else begin\n"
     << "              AWREADY <= 1;\n"
     << "            end\n"
     << "          end\n"
     << "        end\n"
     << "        S_READ: begin\n"
     << "          if (RREADY && RVALID) begin\n"
     << "            rlen <= rlen - 1;\n"
     << "            if (rlen == 0) begin\n"
     << "              st <= S_IDLE;\n"
     << "              access_notify <= 1;\n"
     << "              RLAST <= 0;\n"
     << "              RVALID <= 0;\n"
     << "            end else if (rlen == 1) begin\n"
     << "              RLAST <= 1;\n"
     << "            end\n"
     << "          end else begin\n"
     << "            RVALID <= 1;\n"
     << "          end\n"
     << "          if (RREADY) begin\n"
     << "            sram_addr_src <= sram_addr_src + 1;\n"
     << "          end\n"
     << "          RDATA <= sram_rdata;\n"
     << "        end\n"
     << "        S_WRITE: begin\n"
     << "          if (WVALID) begin\n"
     << "            sram_wdata <= WDATA;\n"
     << "            if (first_addr) begin\n"
     << "              first_addr <= 0;\n"
     << "            end else begin\n"
     << "              sram_addr_src <= sram_addr_src + 1;\n"
     << "            end\n"
     << "          end\n"
     << "          if (WLAST && WVALID) begin\n"
     << "            st <= S_WRITE_DONE;\n"
     << "            BVALID <= 1;\n"
     << "            WREADY <= 0;\n"
     << "          end\n"
     << "        end\n"
     << "        S_READ_AXI: begin\n"
     << "          if (RREADY) begin\n"
     << "            RVALID <= 0;\n"
     << "            RLAST <= 0;\n"
     << "            rlen <= rlen - 1;\n"
     << "            if (rlen == 0) begin\n"
     << "              access_notify <= 1;\n"
     << "              st <= S_IDLE;\n"
     << "            end else begin\n"
     << "              st <= S_READ_SRAM;\n"
     << "              sram_addr_src <= sram_addr_src + 1;\n"
     << "              sram_req <= 1;\n"
     << "            end\n"
     << "          end\n"
     << "        end\n"
     << "        S_READ_SRAM: begin\n"
     << "          if (sram_ack) begin\n"
     << "            sram_req <= 0;\n"
     << "            st <= S_READ_AXI;\n"
     << "            RDATA <= sram_rdata;\n"
     << "            RVALID <= 1;\n"
     << "            if (rlen == 0) begin\n"
     << "              RLAST <= 1;\n"
     << "            end\n"
     << "          end\n"
     << "        end\n"
     << "        S_WRITE_AXI: begin\n"
     << "          if (WVALID) begin\n"
     << "            WREADY <= 0;\n"
     << "            sram_req <= 1;\n"
     << "            sram_wdata <= WDATA;\n"
     << "            last_write <= WLAST;\n"
     << "            st <= S_WRITE_SRAM;\n"
     << "          end\n"
     << "        end\n"
     << "        S_WRITE_SRAM: begin\n"
     << "          if (sram_ack) begin\n"
     << "            sram_req <= 0;\n"
     << "            if (last_write) begin\n"
     << "              st <= S_IDLE;\n"
     << "            end else begin\n"
     << "              sram_addr_src <= sram_addr_src + 1;\n"
     << "              WREADY <= 1;\n"
     << "              st <= S_WRITE_AXI;\n"
     << "            end\n"
     << "          end\n"
     << "        end\n"
     << "        S_WRITE_DONE: begin\n"
     << "          st <= S_IDLE;\n"
     << "          if (BREADY) begin\n"
     << "            BVALID <= 0;\n"
     << "            access_notify <= 1;\n"
     << "          end\n"
     << "        end\n";
  os << "      endcase\n";
}

void SlaveController::AddNotifierPorts() {
  ports_->AddPort("access_notify", Port::OUTPUT, 0);
  ports_->AddPort("access_ack", Port::INPUT, 0);
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
