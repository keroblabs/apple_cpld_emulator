module IOU(
    input           RESET,
    input           PHI_0,
    input           CPU_RW,
    input           C0XX_N,
    input  [7:0]    ADDR,

    input           spi_cs,
    input           spi_clk,
    input           spi_mosi,
    output          spi_miso,

    inout  [7:0]    DATA,
    output          DATA_EN_N,

    output          DEVSEL_n,
    output          PDL_STROBE_n,
    output          C0BX_N,
    output          C0EX_N,

`ifdef IOU_COMPLETE
    output          CAS_OUT,
    output          SPK_OUT,

    output          AN0,
    output          AN1,
    output          AN2,
    output          AN3,
`endif

    input           VIDEO_VLB,
    input           CAS_IN,
    input           SW0,
    input           SW1,
`ifdef IOU_COMPLETE
    input           SW2,
`endif

    input           PDL0,
`ifdef IOU_COMPLETE
    input           PDL1,
    input           PDL2,
    input           PDL3
`else
    input           PDL1
`endif
);

assign          DATA          = CPU_RW ? dbus_out : 8'bzzzzzzzz;
assign          dbus_in       = DATA; //  CPU_RW ? 8'd0 : DATA;
assign          DATA_EN_N     = dbus_en;

`ifdef IOU_COMPLETE
assign          AN3           = AN3_iou;
`endif

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

// source ../../../chip_design/oss-cad-suite/environment
// ../../../chip_design/atf15xx_yosys/run_yosys_IOU.sh IOU
// ../../../chip_design/atf15xx_yosys/run_fitter.sh IOU

IOU_common u_IOU_common (
    // Inputs
    .RESET             (RESET),
    .PHI_0             (PHI_0),
    .CPU_RW            (CPU_RW),
    .C0XX_N            (C0XX_N),
    .ADDR              (ADDR),

    .DHIRES_state_n  (DHIRES_state_n),
    .PDL_STROBE_n    (PDL_STROBE_n),
    .KBDSEL_n        (KBDSEL_n),
    .DEVSEL_n        (DEVSEL_n),
    .C01X_N          (C01X_N),
    .C06X_N          (C06X_N),
    .C0BX_N          (C0BX_N),
    .C0EX_N          (C0EX_N),
    .C0FX_N          (C0FX_N),

    .D7_c01x         (D7_c01x),
    .D7_c06x         (D7_c06x),

`ifdef IOU_COMPLETE
    .CAS_OUT         (CAS_OUT),
    .SPK_OUT         (SPK_OUT),

    .AN0             (AN0),
    .AN1             (AN1),
    .AN2             (AN2),
`endif    
    .AN3             (AN3_iou),

    .KBD_PENDING_IN  (kbd_pending_wire),
    .VIDEO_VLB       (VIDEO_VLB),
    .CAS_IN          (CAS_IN),
    .SW0             (SW0),
    .SW1             (SW1),
`ifdef IOU_COMPLETE
    .SW2             (SW2),
`endif    

    .PDL0            (PDL0),
`ifdef IOU_COMPLETE
    .PDL1            (PDL1),
    .PDL2            (PDL2),
    .PDL3            (PDL3)
`else
    .PDL1            (PDL1)
`endif    
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

endmodule
