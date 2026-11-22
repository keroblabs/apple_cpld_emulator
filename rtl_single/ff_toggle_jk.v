module ff_toggle_jk (
    input       CLK,
    input       RESET,
    input       SET_n,
    input       CLEAR_n,
    output      Q
);

assign Q = pending_r;

reg [1:0]   state_r, state_x;
reg         pending_r, pending_x;

always @(*) begin
    state_x         = state_r;
    pending_x       = pending_r;

    case (state_r)
        2'b00: begin
            if (~SET_n) begin
                state_x     = 2'b01;
                pending_x   = 1'b1;
                //$display("waiting");
            end
        end

        2'b01: begin
            if (~CLEAR_n) begin
                state_x     = 2'b10;
                pending_x   = 1'b0;
                //$display("done");
            end
        end

        2'b10: begin
            if (SET_n) begin
                state_x     = 2'b00;
                //$display("cleaned");
            end
        end

        2'b11: begin
            state_x         = 2'b00;
        end

    endcase
end

always @(posedge CLK) begin
    if (!RESET) begin
        state_r     <= 2'b00;
        pending_r   <= 1'b0;
    end else begin
        state_r     <= state_x;
        pending_r   <= pending_x;
    end
end 

endmodule
