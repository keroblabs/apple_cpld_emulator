// Focused clock-domain test top: the production SPI interface drives the
// production PHI_0-clocked IOMEM pending state machines.
module IOU_spi_phase_tb (
    input         RESET,
    input         PHI_0,
    input         CPU_RW,
    input  [1:0]  ADDR,
    input         C01X_N,
    input         C0FX_N,
    input  [7:0]  dbus_in,
    input         spi_cs,
    input         spi_clk,
    input         spi_mosi,
    output        spi_miso,
    output        wr_pending_state,
    output        rd_pending_state,
    output        kbd_pending_state,
    output        wr_pending_clear_req,
    output        rd_pending_set_req,
    output        kbd_pending_set_req
);

wire [7:0] reg_wr_data;
wire [7:0] reg_rd_data;
wire [6:0] reg_kbd_data;
wire [7:0] dbus_out;
wire       dbus_en;
wire       reg_wr_pending_clr;
wire       reg_rd_pending_set;
wire       reg_kbd_pending_set;

assign wr_pending_clear_req = reg_wr_pending_clr;
assign rd_pending_set_req = reg_rd_pending_set;
assign kbd_pending_set_req = reg_kbd_pending_set;

IOU_iomem iomem (
    .RESET(RESET),
    .PHI_0(PHI_0),
    .CPU_RW(CPU_RW),
    .ADDR(ADDR),
    .D7_c01x(1'b0),
    .D7_c06x(1'b0),
    .AN3(1'b0),
    .C01X_N(C01X_N),
    .C06X_N(1'b1),
    .C0FX_N(C0FX_N),
    .KBDSEL_n(1'b1),
    .DHIRES_state_n(1'b1),
    .dbus_in(dbus_in),
    .dbus_out(dbus_out),
    .dbus_en(dbus_en),
    .reg_wr_data(reg_wr_data),
    .reg_kbd_data(reg_kbd_data),
    .reg_rd_data(reg_rd_data),
    .reg_wr_pending_clr(reg_wr_pending_clr),
    .reg_rd_pending_set(reg_rd_pending_set),
    .reg_kbd_pending_set(reg_kbd_pending_set),
    .reg_wr_pending_state(wr_pending_state),
    .reg_rd_pending_state(rd_pending_state),
    .reg_kbd_pending_state(kbd_pending_state)
);

IOU_spi_iface spi_iface (
    .spi_cs(spi_cs),
    .spi_clk(spi_clk),
    .spi_mosi(spi_mosi),
    .spi_miso(spi_miso),
    .reg_wr_data(reg_wr_data),
    .reg_rd_data(reg_rd_data),
    .reg_kbd_data(reg_kbd_data),
    .reg_wr_pending_clr(reg_wr_pending_clr),
    .reg_rd_pending_set(reg_rd_pending_set),
    .reg_kbd_pending_set(reg_kbd_pending_set),
    .reg_wr_pending_state(wr_pending_state),
    .reg_rd_pending_state(rd_pending_state),
    .reg_kbd_pending_state(kbd_pending_state)
);

endmodule
