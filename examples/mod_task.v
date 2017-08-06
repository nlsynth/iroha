module mod_task(clk, rst_n, req_valid, req_ready, req_0, res_ready, res_valid);
   input clk;
   input rst_n;
   input req_valid;
   output req_ready;
   input [31:0] req_0;
   input res_ready;
   output res_valid;

   reg 	  req_ready;
   reg    res_valid;

endmodule // mod_task
