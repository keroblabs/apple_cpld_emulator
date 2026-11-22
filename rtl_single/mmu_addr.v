module mmu_addr(
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
    input           DMA_N,

    input           CPU_RW,

    output          RAM_EN_N,
    output          ROM_EN_N,
    output          BUS_RD_N,
    output          BUS_WR_N,
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

assign ROM_EN_N             = ~rom_enabled & (onboard_fw_n | ~CPU_RW);
assign RAM_EN_N             = ~ram_enabled;

wire        ram_enabled;
wire        rom_enabled;
wire        bus_phase_valid = RESET & DMA_N & PHI_0;
wire        exp16_ram_en    = CPU_RW ? EXP16_RAM : EXP16_WRITE;
wire        onboard_fw_n    = (IOSEL_N & IOSTB_N) | MODE_SEL;
wire        slot_rom_read   = CPU_RW & ~rom_enabled & (IOSEL_INT_map | IOSTB_INT_map);
wire        flash_read_selected = ~ROM_EN_N | ~IOSEL_N | ~IOSTB_N | ~C6XX_N;

// Existing selects remain the device chip-selects. Shared strobes only
// qualify a granted, high PHI_0 phase. There is not yet an authorised
// flash-program selection in this interface.
assign BUS_RD_N = ~(bus_phase_valid & CPU_RW & (ram_enabled | flash_read_selected));
assign BUS_WR_N = ~(bus_phase_valid & ~CPU_RW & ram_enabled);

// Keep the internal Apple image at its CPU addresses. A slot card sees a
// 4 KiB window, even when MODE_SEL selects the external-card ROM path.
wire [4:0] mem_addr_upper = ram_enabled   ? {addr_16, ADDR[15:13], addr_12} :
                            rom_enabled   ? {1'b0, ADDR[15:12]} :
                            slot_rom_read ? {2'b00, slot_index} : 5'd0;

`ifdef FULL_MMU_ADDR
// IOSEL uses +$000..+$0FF. IOSTB retains the +$800..+$FFF CPU offset.
wire [11:0] mem_addr_lower = (slot_rom_read & IOSEL_INT_map) ? {4'd0, ADDR[7:0]} :
                             (ram_enabled | rom_enabled | slot_rom_read) ? ADDR[11:0] : 12'd0;
assign MEM_ADDR_OUT = {mem_addr_upper, mem_addr_lower};
`else
assign MEM_ADDR_OUT = mem_addr_upper;
`endif

memory_map u_address_signals (
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

mmu_addr16 u_mmu_addr16 (
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

memory_select u_mmu_mem_select (
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
