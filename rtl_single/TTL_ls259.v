module TTL_ls259 (
    input  C_N,
    input  E_N,
    input  D,
    input  [2:0] ADDR,

    output reg Q0, Q1, Q2, Q3, Q4, Q5, Q6, Q7
);

    always @(negedge E_N) begin
        if (!C_N) begin
            Q0 <= 1'b0;
            Q1 <= 1'b0;
            Q2 <= 1'b0;
            Q3 <= 1'b0;
            Q4 <= 1'b0;
            Q5 <= 1'b0;
            Q6 <= 1'b0;
            Q7 <= 1'b0;
        end else begin
            //$display("%d = %d", ADDR, D);
            case (ADDR)
                3'b000: Q0 <= D;
                3'b001: Q1 <= D;
                3'b010: Q2 <= D;
                3'b011: Q3 <= D;
                3'b100: Q4 <= D;
                3'b101: Q5 <= D;
                3'b110: Q6 <= D;
                3'b111: Q7 <= D;
                default: ; // Do nothing
            endcase
        end
    end

endmodule
