module MMU_addr(
    input [15:0]    ADDR,
    input           MODE_SEL,

    input           EXP16_BANK,
    input           EXP16_WRITE,
    input           EXP16_RAM,

    input           SW_RAMRD,
    input           SW_RAMWR,
    input           SW_80STORE,
    input           SW_HIRES,
    input           SW_PAGE2,
    input           SW_ALTZP,

    input           SW_SLOTXROM,
    input           SW_SLOT3ROM,

    input           PHI_0,
    input           RESET,

    input           CPU_RW,

    output          RAM_EN_N,
    output          ROM_EN_N,
    output          IOSEL_N,
    output          IOSTB_N,
    output          C0XX_N,
    output          C6XX_N,

`ifdef FULL_MMU_ADDR
    output [16:0]   MEM_ADDR_OUT
`else
    output [4:0]    MEM_ADDR_OUT
`endif
);

wire addr_12;
wire addr_16;
wire D_FXXX_INT;
wire DXXX_INT;
wire M_0XXX_BXXX_INT;
wire M_00_01XX_INT;
wire M_04_07FF_INT;  
wire M_2_3FFF_INT;  

wire IOSTB_INT_map;
wire IOSEL_INT_map;
wire C0XX_INT;
wire CFFF_INT;

wire[2:0] slot_index;

assign IOSTB_N      = rom_enabled | (~IOSTB_INT_map);
assign IOSEL_N      = rom_enabled | (~(IOSEL_INT_map & C6XX_N));
assign C0XX_N       = ~C0XX_INT;

assign addr_12      = (DXXX_INT & exp16_ram_en) ? EXP16_BANK : ADDR[12];

assign C6XX_N               = ~(ADDR[10] & ADDR[9] & ~ADDR[8] & IOSEL_INT_map);

assign MEM_ADDR_OUT         = mem_addr_out_reg;

assign ROM_EN_N             = ~rom_enabled & onboard_fw_n;
assign RAM_EN_N             = ~ram_enabled;

wire        ram_enabled;
wire        rom_enabled;
wire        exp16_ram_en    = CPU_RW ? EXP16_RAM : EXP16_WRITE;
wire        onboard_fw_n    = (IOSEL_N & IOSTB_N) | MODE_SEL;

`ifdef FULL_MMU_ADDR
reg [16:0]  mem_addr_out_reg;
`else
reg [4:0]   mem_addr_out_reg;
`endif

`ifdef FULL_MMU_ADDR
always @(*) begin
    if( ram_enabled) begin
        mem_addr_out_reg  = { addr_16, ADDR[15:13], addr_12, ADDR[11:0] };
    end else if( rom_enabled & D_FXXX_INT ) begin
        // System ROM (D000-FFFF): raw CPU address, bit16 = 0.
        mem_addr_out_reg  = { 1'b0, ADDR[15:12], ADDR[11:0] };
    end else if( rom_enabled | (~onboard_fw_n) ) begin
        // IO firmware ROM (IOSEL/IOSTB) and external-card path share the
        // per-slot 4 KiB window: { slot_index, ADDR[11:0] }. First 2 KiB of
        // the window holds the IOSEL ($Cn00-$CnFF) page; the second 2 KiB
        // holds the IOSTB expansion ($C800-$CFFF) for the selected slot.
        mem_addr_out_reg  = { 2'b00, slot_index, ADDR[11:0] };
    end else begin
        mem_addr_out_reg  = 17'd0;
    end
end
`else
always @(*) begin
    if( ram_enabled) begin
        mem_addr_out_reg  = { addr_16, ADDR[15:13], addr_12 };
    end else if( rom_enabled & D_FXXX_INT ) begin
        // System ROM (D000-FFFF): raw upper bits.
        mem_addr_out_reg  = { 1'b0, ADDR[15:12] };
    end else if( rom_enabled | (~onboard_fw_n) ) begin
        // IO firmware ROM / external-card: per-slot 4 KiB window select.
        // The lower 12 bits (within-window offset = ADDR[11:0]) are
        // reconstructed from the CPU address bus in the default build.
        mem_addr_out_reg  = { 2'b00, slot_index };
    end else begin
        mem_addr_out_reg  = 5'b0;
    end
end
`endif

MMU_mem_map u_address_signals (
    .ADDR               (ADDR),
    .D_FXXX_INT         (D_FXXX_INT),
    .DXXX_INT           (DXXX_INT),
    .IOSTB_INT          (IOSTB_INT_map),
    .IOSEL_INT          (IOSEL_INT_map),
    .CFFF_INT           (CFFF_INT),
    .C0XX_INT           (C0XX_INT),

    .M_0XXX_BXXX_INT    (M_0XXX_BXXX_INT),
    .M_00_01XX_INT      (M_00_01XX_INT),  
    .M_04_07FF_INT      (M_04_07FF_INT),  
    .M_2_3FFF_INT       (M_2_3FFF_INT)        
);

MMU_addr16 u_mmu_addr16 (
    .PHI_0              (PHI_0),
    .RESET              (RESET),
    .CPU_RW             (CPU_RW),

    .M_0XXX_BXXX_INT    (M_0XXX_BXXX_INT),
    .M_00_01XX_INT      (M_00_01XX_INT),  
    .M_04_07FF_INT      (M_04_07FF_INT),  
    .M_2_3FFF_INT       (M_2_3FFF_INT),     
    .D_FXXX_INT         (D_FXXX_INT),
    .exp16_ram_en       (exp16_ram_en),

    .SW_RAMRD           (SW_RAMRD),
    .SW_RAMWR           (SW_RAMWR),
    .SW_80STORE         (SW_80STORE),
    .SW_HIRES           (SW_HIRES),
    .SW_PAGE2           (SW_PAGE2),
    .SW_ALTZP           (SW_ALTZP),

    .addr_16_out        (addr_16)
);

MMU_mem_select u_mmu_mem_select (
    .PHI_0            (PHI_0),
    .RESET            (RESET),
    .CPU_RW           (CPU_RW),
    .ADDR             (ADDR),

    .D_FXXX_INT       (D_FXXX_INT),
    .CFFF_INT         (CFFF_INT),
    .IOSTB_INT        (IOSTB_INT_map),
    .IOSEL_INT        (IOSEL_INT_map),
    .M_0XXX_BXXX_INT  (M_0XXX_BXXX_INT),

    .SW_SLOTXROM      (SW_SLOTXROM),
    .SW_SLOT3ROM      (SW_SLOT3ROM),
    .exp16_ram_en     (exp16_ram_en),

    .slot_index       (slot_index),
    .ram_enabled      (ram_enabled),
    .rom_enabled      (rom_enabled)
);

endmodule
