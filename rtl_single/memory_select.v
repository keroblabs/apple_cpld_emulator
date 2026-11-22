module memory_select(
    input           PHI_0,
    input           RESET,
    input           CPU_RW,
    input [15:0]    ADDR, // ADDR[15:8]

    input           D_FXXX_INT,
    input           CFFF_INT,
    input           IOSTB_INT,
    input           IOSEL_INT,
    input           M_0XXX_BXXX_INT,

    input           SW_SLOTXROM,
    input           SW_SLOT3ROM,
    input           exp16_ram_en,

    output [2:0]    slot_index,
    output          ram_enabled,
    output          rom_enabled
);

slot_control U_slot_control (
    .PHI_0          (PHI_0),
    .RESET          (RESET),
    .IOSEL_INT      (IOSEL_INT),
    .CFFF_INT       (CFFF_INT),
    .SW_SLOTXROM    (SW_SLOTXROM),
    .SW_SLOT3ROM    (SW_SLOT3ROM),
    .ADDR_10_8      (ADDR[10:8]),
    .slot_index     (slot_index),
    .rom_fw_enable  (rom_fw_enable)
);

wire [4:0] addr_unused_0    = ADDR[15:11];
wire [7:0] addr_unused_1    = ADDR[7:0];

wire   rom_fw_enable;
wire   ram_enabled_int   = M_0XXX_BXXX_INT | (D_FXXX_INT & exp16_ram_en);
wire   rom_enabled_int   = CPU_RW & ((IOSEL_INT & rom_fw_enable) |
                                     (IOSTB_INT & rom_fw_enable) |
                                     (D_FXXX_INT & ~exp16_ram_en));

assign ram_enabled = ram_enabled_int;
assign rom_enabled = rom_enabled_int;

//always @(*) begin
//  $display("ROM/RAM = %d %d", rom_enabled_int, ram_enabled_int);
//end

endmodule
