module mod_hello(clk, rst_n, req_valid, req_ready, x, y, z);
   input clk;
   input rst_n;
   input req_valid;
   output req_ready;
   input [31:0] x;
   input [31:0] y;
   output z;

   reg 	  req_ready;
   reg 	  done;
   reg 	  z;

   always @(posedge clk) begin
      if (!rst_n) begin
	 done <= 0;
      end else begin
	 if (req_valid) begin
	    if (!done) begin
		$display("Hello world!");
		done <= 1;
	    end
	 end
	 req_ready <= req_valid;
      end
   end
endmodule
