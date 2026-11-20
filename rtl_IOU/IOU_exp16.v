module IOU_exp16 (
    input           C08X,
    input           PHI_0,
    input           RESET,
    input [3:0]     ADDR,

    output          EXP16_BANK,
    output          EXP16_RAM,
    output          EXP16_WRITE
);

reg         exp16_bank_r,   exp16_bank_x;
reg         exp16_ram_r,    exp16_ram_x;
reg [1:0]   exp16_write_r,  exp16_write_x;

assign EXP16_BANK   = exp16_bank_r;
assign EXP16_RAM    = exp16_ram_r;
assign EXP16_WRITE  = exp16_write_r[1];

wire addr_2_unused = ADDR[2];

always @(*) begin
    exp16_bank_x    = exp16_bank_r;
    exp16_ram_x     = exp16_ram_r;
    exp16_write_x   = exp16_write_r;

    if (!C08X) begin
        exp16_bank_x        = ~ADDR[3];
        exp16_ram_x         = ~(ADDR[1] ^ ADDR[0]);

        if (ADDR[0]) begin
            exp16_write_x[1] = exp16_write_r[0];
            exp16_write_x[0] = 1'b1;
        end else begin
            exp16_write_x    = 2'b00;
        end
    end
end

always @(negedge PHI_0) begin
    if (~RESET) begin
        exp16_bank_r    <= 1'b0;
        exp16_ram_r     <= 1'b0;
        exp16_write_r   <= 2'b00;
    end else begin
        exp16_bank_r    <= exp16_bank_x;
        exp16_ram_r     <= exp16_ram_x;
        exp16_write_r   <= exp16_write_x;
    end
end

endmodule
