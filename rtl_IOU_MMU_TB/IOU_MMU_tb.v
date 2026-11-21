module IOU_MMU_tb(
    input           RESET,
    input           PHI_0,
    input           CPU_RW,
    input  [15:0]   ADDR,

    input           DMA_N,
    input           MODE_SEL,
    output          PHIO_0,
    output          BUS_N,

    input           spi_cs,
    input           spi_clk,
    input           spi_mosi,
    output          spi_miso,

    inout  [7:0]    DATA,
    output          DATA_EN_N,

`ifdef FULL_MMU_ADDR
    output [16:0]   MEM_ADDR_OUT,
`else
    output [4:0]    MEM_ADDR_OUT,
`endif
    output          RAM_EN_N,
    output          ROM_EN_N,
    output          BUS_RD_N,
    output          BUS_WR_N,

    output          DEVSEL_n,
    output          PDL_STROBE_n,
    output          C0BX_N,
    output          C0EX_N,
    output          IOSEL_OUT_N,
    output          C6XX_N,
    output          IOSTB_N,

    output          CAS_OUT,
    output          SPK_OUT,

    output          AN0,
    output          AN1,
    output          AN2,
    output          AN3,

    input           VIDEO_VLB,
    input           CAS_IN,
    input           SW0,
    input           SW1,
    input           SW2,

    input           PDL0,
    input           PDL1,
    input           PDL2,
    input           PDL3
);

assign          DATA          = CPU_RW ? dbus_out : 8'bzzzzzzzz;
assign          dbus_in       = DATA; //  CPU_RW ? 8'd0 : DATA;
assign          DATA_EN_N     = dbus_en;
assign          AN3           = AN3_iou;

assign          PHIO_0        = PHI_0 & DMA_N;
assign          BUS_N         = ~DMA_N;

wire  [7:0]     dbus_in;
wire  [7:0]     dbus_out;

wire            AN3_iou;

wire            dbus_en;

wire            D7_c01x;
wire            D7_c06x;

wire            DHIRES_state_n;
wire            KBDSEL_n;
wire            C01X_N;
wire            C06X_N;
wire            C0FX_N;

wire [7:0]      reg_wr_data;

wire  [6:0]     reg_kbd_data;
wire  [7:0]     reg_rd_data;

wire            reg_wr_pending_clr;

wire            reg_rd_pending_set;
wire            reg_kbd_pending_set;

wire            reg_wr_pending_state;
wire            reg_rd_pending_state;
wire            kbd_pending_wire;

wire            EXP16_BANK;
wire            EXP16_RAM;
wire            EXP16_WRITE;

wire            SW_PAGE2;
wire            SW_HIRES;
wire            SW_RAMRD;
wire            SW_RAMWR;
wire            SW_ALTZP;
wire            SW_SLOTXROM;
wire            SW_SLOT3ROM;
wire            SW_80STORE;

wire            C0XX_N;

IOU_common u_IOU_common (
    // Inputs
    .RESET              (RESET),
    .PHI_0              (PHI_0),
    .CPU_RW             (CPU_RW),
    .C0XX_N             (C0XX_N),
    .ADDR               (ADDR[7:0]),

    .DHIRES_state_n     (DHIRES_state_n),
    .PDL_STROBE_n       (PDL_STROBE_n),
    .KBDSEL_n           (KBDSEL_n),
    .DEVSEL_n           (DEVSEL_n),
    .C01X_N             (C01X_N),
    .C06X_N             (C06X_N),
    .C0BX_N             (C0BX_N),
    .C0EX_N             (C0EX_N),
    .C0FX_N             (C0FX_N),

    .CAS_OUT            (CAS_OUT),
    .SPK_OUT            (SPK_OUT),

    .D7_c01x            (D7_c01x),
    .D7_c06x            (D7_c06x),

    .AN0                (AN0),
    .AN1                (AN1),
    .AN2                (AN2),
    .AN3                (AN3_iou),

    .KBD_PENDING_IN     (kbd_pending_wire),
    .VIDEO_VLB          (VIDEO_VLB),
    .CAS_IN             (CAS_IN),
    .SW0                (SW0),
    .SW1                (SW1),
    .SW2                (SW2),

    .PDL0               (PDL0),
    .PDL1               (PDL1),
    .PDL2               (PDL2),
    .PDL3               (PDL3)
);

IOU_iomem iomem_instance (
    .RESET                     (RESET),
    .PHI_0                     (PHI_0),
    .CPU_RW                    (CPU_RW),
    .ADDR                      (ADDR[1:0]),

    .D7_c01x                   (D7_c01x),
    .D7_c06x                   (D7_c06x),
    .AN3                       (AN3_iou),    

    .C01X_N                    (C01X_N),
    .C06X_N                    (C06X_N),
    .C0FX_N                    (C0FX_N),
    .KBDSEL_n                  (KBDSEL_n),
    .DHIRES_state_n            (DHIRES_state_n),

    .dbus_in                   (dbus_in),
    .dbus_out                  (dbus_out),

    .dbus_en                   (dbus_en),

    .reg_wr_data               (reg_wr_data),

    .reg_kbd_data              (reg_kbd_data),
    .reg_rd_data               (reg_rd_data),

    .reg_wr_pending_clr        (reg_wr_pending_clr),

    .reg_rd_pending_set        (reg_rd_pending_set),
    .reg_kbd_pending_set       (reg_kbd_pending_set),

    .reg_wr_pending_state      (reg_wr_pending_state),
    .reg_rd_pending_state      (reg_rd_pending_state),
    .reg_kbd_pending_state     (kbd_pending_wire)
);

IOU_spi_iface u_iou_spi_iface (
    .spi_cs                 (spi_cs),
    .spi_clk                (spi_clk),
    .spi_mosi               (spi_mosi),
    .spi_miso               (spi_miso),

    .reg_wr_data            (reg_wr_data),

    .reg_rd_data            (reg_rd_data),
    .reg_kbd_data           (reg_kbd_data),

    .reg_wr_pending_clr     (reg_wr_pending_clr),

    .reg_rd_pending_set     (reg_rd_pending_set),
    .reg_kbd_pending_set    (reg_kbd_pending_set),

    .reg_wr_pending_state   (reg_wr_pending_state),
    .reg_rd_pending_state   (reg_rd_pending_state),
    .reg_kbd_pending_state  (kbd_pending_wire)
);


MMU_addr u_MMU_addr (
    .ADDR            (ADDR),
    .PHI_0           (PHI_0),
    .RESET           (RESET), 
    .DMA_N           (DMA_N),
    .CPU_RW          (CPU_RW),
    .MODE_SEL        (MODE_SEL),

    .EXP16_BANK      (EXP16_BANK),
    .EXP16_WRITE     (EXP16_WRITE),
    .EXP16_RAM       (EXP16_RAM),

    .SW_SLOTXROM     (SW_SLOTXROM),
    .SW_SLOT3ROM     (SW_SLOT3ROM),
    .SW_RAMRD        (SW_RAMRD),
    .SW_RAMWR        (SW_RAMWR),
    .SW_80STORE      (SW_80STORE),
    .SW_ALTZP        (SW_ALTZP),
    .SW_HIRES        (SW_HIRES),
    .SW_PAGE2        (SW_PAGE2),

    .RAM_EN_N        (RAM_EN_N),
    .ROM_EN_N        (ROM_EN_N),
    .BUS_RD_N        (BUS_RD_N),
    .BUS_WR_N        (BUS_WR_N),

    .IOSEL_N         (IOSEL_OUT_N),
    .IOSTB_N         (IOSTB_N),
    .C0XX_N          (C0XX_N),
    .C6XX_N          (C6XX_N),

    .MEM_ADDR_OUT    (MEM_ADDR_OUT)
);

MMU_io u_MMU_io (
    // Inputs
    .RESET             (RESET),
    .PHI_0             (PHI_0),
    .CPU_RW            (CPU_RW),
    .C0XX_N            (C0XX_N),
    .ADDR              (ADDR[7:0]),

    .EXP16_BANK      (EXP16_BANK),
    .EXP16_RAM       (EXP16_RAM),
    .EXP16_WRITE     (EXP16_WRITE),

    .SW_RAMRD        (SW_RAMRD),
    .SW_RAMWR        (SW_RAMWR),
    .SW_SLOTXROM     (SW_SLOTXROM),
    .SW_SLOT3ROM     (SW_SLOT3ROM),
    .SW_ALTZP        (SW_ALTZP),
    .SW_80STORE      (SW_80STORE),

    .SW_PAGE2        (SW_PAGE2),
    .SW_HIRES        (SW_HIRES)
);

endmodule
