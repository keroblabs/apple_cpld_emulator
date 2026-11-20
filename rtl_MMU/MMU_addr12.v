module MMU_addr12(
    input               exp16_ram_en,
    input               EXP16_BANK,

    input               DXXX_INT,
    input               ADDR_12_in,

    output              ADDR_12_out
);

wire    addr_12         = (DXXX_INT & exp16_ram_en) ? EXP16_BANK : ADDR_12_in;

assign  ADDR_12_out     = addr_12;

endmodule
