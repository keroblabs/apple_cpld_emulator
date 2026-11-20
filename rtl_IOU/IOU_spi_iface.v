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

assign      spi_miso        = tx_shifer_r[7];

assign      reg_rd_data     = reg_rd_data_int;
assign      reg_kbd_data    = reg_kbd_data_int;

reg [7:0]    reg_rd_data_int;
reg [6:0]    reg_kbd_data_int;

reg [3:0]   tx_bit_count_r,     tx_bit_count_x;
reg [7:0]   tx_shifer_r,        tx_shifer_x;
reg [10:0]  rx_shifer_r,        rx_shifer_x;

always @(*) begin
    reg_wr_pending_clr          = 1'b1;
    reg_rd_pending_set          = 1'b1;
    reg_kbd_pending_set         = 1'b1;

    if (~spi_cs && (tx_bit_count_r == 4'b1101)) begin
        //$display("load %04x", rx_shifer_r);
        if (rx_shifer_r[10:8] == 3'd0) begin
            reg_wr_pending_clr      = 1'b0;
        end else if (rx_shifer_r[10:8] == 3'd1) begin
            reg_rd_pending_set      = 1'b0;
        end else if (rx_shifer_r[10:8] == 3'd2) begin
            //$display("KBD load");
            reg_kbd_pending_set     = 1'b0;
        end
    end
end

always @(*) begin
    tx_bit_count_x              = tx_bit_count_r;

    tx_shifer_x                 = tx_shifer_r;
    rx_shifer_x                 = rx_shifer_r;

    if (~spi_cs) begin
        rx_shifer_x[10:1]       = rx_shifer_r[9:0];
        rx_shifer_x[0]          = spi_mosi;

        tx_shifer_x[7:1]        = tx_shifer_r[6:0];
        tx_shifer_x[0]          = 1'b0;
        tx_bit_count_x          = tx_bit_count_r + 1'b1;
    end
end

always @(posedge spi_clk) begin
    if (spi_cs) begin
        //$display("SPI reset %d", reg_kbd_pending_state);
        tx_bit_count_r          <= 4'd0;
        
        rx_shifer_r             <= 11'd0;
        tx_shifer_r             <= { 2'b00,
                                     reg_kbd_pending_state,
                                     reg_rd_pending_state,
                                     reg_wr_pending_state,
                                     3'b000 };
    end else begin
        tx_bit_count_r          <= tx_bit_count_x;

        rx_shifer_r             <= rx_shifer_x;
        tx_shifer_r             <= tx_shifer_x;

        //$display("bit [%d] %d %d", tx_bit_count_x, rx_shifer_x[0], tx_shifer_x[7]);

        if (tx_bit_count_x == 4'b0101) begin
            //$display("addr %02x", rx_shifer_x);
            if (rx_shifer_x[2:0] == 3'd0) begin
                tx_shifer_r             <= reg_wr_data;
                //$display("APL %02x", reg_wr_data);
            end else begin
                tx_shifer_r             <= 8'b11111111;
            end
        end else if (tx_bit_count_x == 4'b1101) begin
            if (rx_shifer_x[10:8] == 3'd1) begin
                reg_rd_data_int         <= rx_shifer_x[7:0];
            end else if (rx_shifer_x[10:8] == 3'd2) begin
                reg_kbd_data_int        <= rx_shifer_x[6:0];
            end
        end
    end
end

endmodule
