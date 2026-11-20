module IOU_toggle_d (
    input       PHI_0,
    input       CS,
    input       RESET,
    output      Q
);

reg state_r, state_x;

assign Q = state_r;

always @(*) begin
    state_x         = state_r;

    if (!CS) begin
        state_x     = ~state_r;
    end
end

always @(negedge PHI_0) begin
    if (!RESET) begin
        state_r     <= 1'b1;
    end else begin
        state_r     <= state_x;
    end
end 

endmodule
