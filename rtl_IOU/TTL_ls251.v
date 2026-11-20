module TTL_ls251 (
    input        D0,
    input        D1,
    input        D2,
    input        D3,
    input        D4,
    input        D5,
    input        D6,
    input        D7,

    input  [2:0] ADDR,
    input        ENABLE,

    output       Q
);

    // Internal signal for the output before 3-state control
    reg internal_data_out;
    reg d_out;

    assign Q = d_out;

    // Logic for data selection
    always @(*) begin
        case (ADDR)
            3'b000: internal_data_out = D0;
            3'b001: internal_data_out = D1;
            3'b010: internal_data_out = D2;
            3'b011: internal_data_out = D3;
            3'b100: internal_data_out = D4;
            3'b101: internal_data_out = D5;
            3'b110: internal_data_out = D6;
            3'b111: internal_data_out = D7;
            default: internal_data_out = 1'b0; // Default case for safety
        endcase
    end

    // 3-state output control
    always @(*) begin
        if (ENABLE == 1'b0) begin
            d_out = internal_data_out;
        end else begin
            d_out = 1'b0; // High impedance when strobe is high
        end
    end

endmodule
