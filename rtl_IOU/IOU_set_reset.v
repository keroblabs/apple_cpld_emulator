module IOU_set_reset (
    input       RESET,
    input       SET_n,
    input       CLEAR_n,
    output      Q    
);

assign Q = out;

// To properly detect falling edges, we need to store previous states
reg prev_set, prev_clear, out;

always @(negedge RESET or negedge SET_n or negedge CLEAR_n) begin
    if (!RESET) begin
        out <= 1'b0;
        prev_set <= SET_n;
        prev_clear <= CLEAR_n;
    end else begin
        // Detect falling edge of set
        if (prev_set && ~SET_n) begin
            out <= 1'b1;
        end
        // Detect falling edge of clear
        if (prev_clear && ~CLEAR_n) begin
            out <= 1'b0;
        end
        // Update previous states
        prev_set <= SET_n;
        prev_clear <= CLEAR_n;
    end
end

endmodule
