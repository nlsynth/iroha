module mod_hello(clk, rst_n, req_hello, ack_hello, arg_hello);
   input clk;
   input rst_n;
   input req_hello;
   input [31:0] arg_hello;
   output ack_hello;

   reg 	  ack_hello;
   reg 	  done;

   always @(posedge clk) begin
      if (!rst_n) begin
	 done <= 0;
      end else begin
	 if (req_hello) begin
	    if (!done) begin
		$display("Hello world [%d]", arg_hello);
		done <= 1;
	    end
	 end
	 ack_hello <= req_hello;
      end
   end
endmodule
