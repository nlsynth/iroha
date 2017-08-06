module mod_task(clk, rst_n, req_valid, req_ready, req_0, res_ready, res_valid, res_0);
   input clk;
   input rst_n;
   input req_valid;
   output req_ready;
   input [31:0] req_0;
   input res_ready;
   output res_valid;
   output [31:0] res_0;

   reg 	  req_ready;
   reg    res_valid;
   reg [31:0] res_0;

endmodule // mod_task
