module mod_flow(clk, rst_n, req_0, res_0);

   input clk;
   input rst_n;
   input [31:0] req_0;
   output [31:0] res_0;

   reg [31:0] res_0;
   

   always @(posedge clk) begin
      if (!rst_n) begin
      end else begin
	 res_0 <= req_0 + 1;
      end
   end

endmodule // mod_flow
