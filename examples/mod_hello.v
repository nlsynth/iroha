module mod_hello(clk, rst_n, req_valid, req_ready);
   input clk;
   input rst_n;
   input req_valid;
   output req_ready;

   reg 	  req_ready;
   reg 	  done;

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
