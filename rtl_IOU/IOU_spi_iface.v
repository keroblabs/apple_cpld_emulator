module IOU_spi_iface (
    input           spi_cs,
    input           spi_clk,
    input           spi_mosi,
    output          spi_miso,

    input  [7:0]    reg_wr_data,

    output [7:0]    reg_rd_data,
    output [6:0]    reg_kbd_data,

    output reg      reg_wr_pending_clr,

    output reg      reg_rd_pending_set,
    output reg      reg_kbd_pending_set,

    input           reg_wr_pending_state,
    input           reg_rd_pending_state,
    input           reg_kbd_pending_state
);

assign spi_miso    = tx_shifter_r[7];
assign reg_rd_data = reg_rd_data_int;
assign reg_kbd_data = reg_kbd_data_int;

reg [7:0]  reg_rd_data_int;
reg [6:0]  reg_kbd_data_int;

reg [3:0]  bit_count_r, bit_count_x;
reg [7:0]  tx_shifter_r, tx_shifter_x;
reg [12:0] rx_shifter_r, rx_shifter_x;

always @(*) begin
    reg_wr_pending_clr  = 1'b1;
    reg_rd_pending_set = 1'b1;
    reg_kbd_pending_set = 1'b1;

    if (~spi_cs && (bit_count_r == 4'd13)) begin
        case (rx_shifter_r[12:8])
            5'd0: reg_wr_pending_clr  = 1'b0;
            5'd1: reg_rd_pending_set = 1'b0;
            5'd2: reg_kbd_pending_set = 1'b0;
            default: begin
            end
        endcase
    end
end

always @(*) begin
    bit_count_x = bit_count_r;
    tx_shifter_x = tx_shifter_r;
    rx_shifter_x = rx_shifter_r;

    // Accept thirteen payload/frame edges and the legacy trailing edge.
    // Saturating at fourteen prevents extra SCK edges from wrapping the
    // four-bit counter and replaying a command while CS remains asserted.
    if (~spi_cs && (bit_count_r < 4'd14)) begin
        rx_shifter_x = {rx_shifter_r[11:0], spi_mosi};
        tx_shifter_x = {tx_shifter_r[6:0], 1'b0};
        bit_count_x = bit_count_r + 1'b1;

        // Five selector bits arrive before the payload. Only selector zero
        // returns the Apple->controller mailbox byte on MISO.
        if (bit_count_x == 4'd5) begin
            if (rx_shifter_x[4:0] == 5'd0)
                tx_shifter_x = reg_wr_data;
            else
                tx_shifter_x = 8'hFF;
        end
    end
end

always @(posedge spi_clk) begin
    if (spi_cs) begin
        bit_count_r <= 4'd0;
        rx_shifter_r <= 13'd0;
        tx_shifter_r <= {2'b00,
                         reg_kbd_pending_state,
                         reg_rd_pending_state,
                         reg_wr_pending_state,
                         3'b000};
    end else if (bit_count_r < 4'd14) begin
        bit_count_r <= bit_count_x;
        rx_shifter_r <= rx_shifter_x;
        tx_shifter_r <= tx_shifter_x;

        if (bit_count_x == 4'd13) begin
            case (rx_shifter_x[12:8])
                5'd1: reg_rd_data_int <= rx_shifter_x[7:0];
                5'd2: reg_kbd_data_int <= rx_shifter_x[6:0];
                default: begin
                end
            endcase
        end
    end
end

endmodule
