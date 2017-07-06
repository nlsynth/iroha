#include "writer/verilog/axi/slave_controller.h"

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
  string name = SlavePort::ControllerName(res_, reset_polarity_);
  os << "// slave controller: "
     << name << "\n";
  AddSramPorts();
  string initials;
  GenReadChannel(cfg_, false, nullptr, ports_.get(), &initials);
  GenWriteChannel(cfg_, false, nullptr, ports_.get(), &initials);
  os << "module " << name << "(";
  ports_->Output(Ports::PORT_NAME, os);
  os << ");\n";
  ports_->Output(Ports::PORT_TYPE, os);

  os << "  `define S_IDLE 0\n"
     << "  // single cycle mode (sram_EXCLUSIVE)\n"
     << "  `define S_READ 1\n"
     << "  `define S_WRITE 2\n"
     << "  // multi cycle mode (!sram_EXCLUSIVE)\n"
     << "  `define S_READ_AXI 3\n"
     << "  `define S_READ_SRAM 4\n"
     << "  `define S_WRITE_AXI 5\n"
     << "  `define S_WRITE_SRAM 6\n";
  os << "  reg [3:0] st;\n\n";
  os << "  reg [" << sram_addr_width_ << ":0] idx;\n\n";
  os << "  reg first_addr;\n"
     << "  reg last_write;\n"
     << "  reg [7:0] rlen;\n\n";

  os << "  always @(posedge clk) begin\n"
     << "    if (" << (reset_polarity_ ? "" : "!")
     << ResetName(reset_polarity_) << ") begin\n"
     << "      st <= `S_IDLE;\n"
     << "      first_addr <= 0;\n"
     << "      last_write <= 0;\n"
     << "      rlen <= 0;\n";
  os << initials
     << "    end else begin\n";
  OutputFSM(os);
  os << "    end\n"
     << "  end\n"
     << "endmodule\n";
}

void SlaveController::AddPorts(const PortConfig &cfg, Module *mod, string *s) {
  Ports *ports = mod->GetPorts();
  GenWriteChannel(cfg, false, mod, ports, s);
  GenReadChannel(cfg, false, mod, ports, s);
}

void SlaveController::OutputFSM(ostream &os) {
  os << "      sram_wen <= (st == `S_WRITE && WVALID);\n"
     << "      case (st)\n"
     << "        `S_IDLE: begin\n"
     << "          if (ARVALID) begin\n"
     << "            if (ARREADY) begin\n"
     << "              ARREADY <= 0;\n"
     << "              sram_addr <= ARADDR[" << (sram_addr_width_ - 1) << ":0];\n"
     << "              rlen <= ARLEN;\n"
     << "              if (sram_EXCLUSIVE) begin\n"
     << "                st <= `S_READ;\n"
     << "              end else begin\n"
     << "                st <= `S_READ_SRAM;\n"
     << "                sram_req <= 1;\n"
     << "              end\n"
     << "            end else begin\n"
     << "              ARREADY <= 1;\n"
     << "            end\n"
     << "          end else if (AWVALID) begin\n"
     << "            if (AWREADY) begin\n"
     << "              AWREADY <= 0;\n"
     << "              first_addr <= 1;\n"
     << "              sram_addr <= AWADDR[" << (sram_addr_width_ - 1) << ":0];\n"
     << "              if (sram_EXCLUSIVE) begin\n"
     << "                st <= `S_WRITE;\n"
     << "              end else begin\n"
     << "                st <= `S_WRITE_AXI;\n"
     << "              end\n"
     << "              WREADY <= 1;\n"
     << "            end else begin\n"
     << "              AWREADY <= 1;\n"
     << "            end\n"
     << "          end\n"
     << "        end\n"
     << "        `S_READ: begin\n"
     << "          if (RREADY && RVALID) begin\n"
     << "            sram_addr <= sram_addr + 1;\n"
     << "            rlen <= rlen - 1;\n"
     << "            if (rlen == 0) begin\n"
     << "              st <= `S_IDLE;\n"
     << "              RLAST <= 0;\n"
     << "              RVALID <= 0;\n"
     << "            end\n"
     << "          end else begin\n"
     << "            RVALID <= 1;\n"
     << "            if (rlen == 0) begin\n"
     << "              RLAST <= 1;\n"
     << "            end\n"
     << "          end\n"
     << "          RDATA <= sram_rdata;\n"
     << "        end\n"
     << "        `S_WRITE: begin\n"
     << "          if (WVALID) begin\n"
     << "            sram_wdata <= WDATA;\n"
     << "            if (first_addr) begin\n"
     << "              first_addr <= 0;\n"
     << "            end else begin\n"
     << "              sram_addr <= sram_addr + 1;\n"
     << "            end\n"
     << "          end\n"
     << "          if (WLAST && WVALID) begin\n"
     << "            st <= `S_IDLE;\n"
     << "            WREADY <= 0;\n"
     << "          end\n"
     << "        end\n"
     << "        `S_READ_AXI: begin\n"
     << "          if (RREADY) begin\n"
     << "            RVALID <= 0;\n"
     << "            RLAST <= 0;\n"
     << "            rlen <= rlen - 1;\n"
     << "            if (rlen == 0) begin\n"
     << "              st <= `S_IDLE;\n"
     << "            end else begin\n"
     << "              st <= `S_READ_SRAM;\n"
     << "              sram_addr <= sram_addr + 1;\n"
     << "              sram_req <= 1;\n"
     << "            end\n"
     << "          end\n"
     << "        end\n"
     << "        `S_READ_SRAM: begin\n"
     << "          if (sram_ack) begin\n"
     << "            sram_req <= 0;\n"
     << "            st <= `S_READ_AXI;\n"
     << "            RDATA <= sram_rdata;\n"
     << "            RVALID <= 1;\n"
     << "            if (rlen == 0) begin\n"
     << "              RLAST <= 1;\n"
     << "            end\n"
     << "          end\n"
     << "        end\n"
     << "        `S_WRITE_AXI: begin\n"
     << "          if (WVALID) begin\n"
     << "            WREADY <= 0;\n"
     << "            sram_req <= 1;\n"
     << "            sram_wdata <= WDATA;\n"
     << "            last_write <= WLAST;\n"
     << "            st <= `S_WRITE_SRAM;\n"
     << "          end\n"
     << "        end\n"
     << "        `S_WRITE_SRAM: begin\n"
     << "          if (sram_ack) begin\n"
     << "            sram_req <= 0;\n"
     << "            if (last_write) begin\n"
     << "              st <= `S_IDLE;\n"
     << "            end else begin\n"
     << "              sram_addr <= sram_addr + 1;\n"
     << "              WREADY <= 1;\n"
     << "            end\n"
     << "          end\n"
     << "        end\n";
  os << "      endcase\n";
}

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
