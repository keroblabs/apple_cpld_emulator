module TTL_ls251_2 (
    input        D0,
    input        D1,
    input        D2,
    input        D3,
    input        D4,
    input        D5,
    input        D6,
    input        D7,
    input        D8,
    input        D9,
    input        D10,
    input        D11,
    input        D12,
    input        D13,
    input        D14,
    input        D15,

    input  [3:0] ADDR,
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
            4'b0000: internal_data_out = D0;
            4'b0001: internal_data_out = D1;
            4'b0010: internal_data_out = D2;
            4'b0011: internal_data_out = D3;
            4'b0100: internal_data_out = D4;
            4'b0101: internal_data_out = D5;
            4'b0110: internal_data_out = D6;
            4'b0111: internal_data_out = D7;
            4'b1000: internal_data_out = D8;
            4'b1001: internal_data_out = D9;
            4'b1010: internal_data_out = D10;
            4'b1011: internal_data_out = D11;
            4'b1100: internal_data_out = D12;
            4'b1101: internal_data_out = D13;
            4'b1110: internal_data_out = D14;
            4'b1111: internal_data_out = D15;
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
