module iomem_block(
    input           RESET,
    input           PHI_0,
    input           CPU_RW,
    input  [1:0]    ADDR,

    input           D7_c01x,
    input           D7_c06x,
    input           AN3,    

    input           C01X_N,
    input           C06X_N,
    input           C0FX_N,
    input           KBDSEL_n,
    input           DHIRES_state_n,

    input  [7:0]    dbus_in,
    output [7:0]    dbus_out,

    output          dbus_en,

    ///////////////////////////////////////

    output [7:0]    reg_wr_data,

    input  [6:0]    reg_kbd_data,
    input  [7:0]    reg_rd_data,
    
    input           reg_wr_pending_clr,

    input           reg_rd_pending_set,
    input           reg_kbd_pending_set,

    output          reg_wr_pending_state,
    output          reg_rd_pending_state,
    output          reg_kbd_pending_state
);

assign          dbus_out            = dbus_out_r;
assign          reg_wr_data         = reg_out_r;
assign          dbus_en             = dbus_en_int;

assign          reg_wr_pending_state  = reg_wr_pending;
assign          reg_rd_pending_state  = reg_rd_pending;
assign          reg_kbd_pending_state = reg_kbd_pending;


reg [7:0]       reg_out_r,          reg_out_x;
reg [7:0]       dbus_out_r,         dbus_out_x;

wire        reg_wr_pending;
wire        reg_rd_pending;
wire        reg_kbd_pending;

reg             reg_wr_pending_set;   /*reg_wr_pending_clr,*/
reg           /*reg_rd_pending_set,*/   reg_rd_pending_clr;
reg           /*reg_kbd_pending_set,*/  reg_kbd_pending_clr;

reg             dbus_en_int;
reg             D7_status_line;
reg             D7_status_en        = ~(C01X_N & C06X_N & DHIRES_state_n);

// IOU_toggle_jk
// IOU_set_reset
ff_toggle_jk ff_toggle_wr (
    .CLK        (PHI_0),
    .RESET      (RESET),
    .SET_n      (reg_wr_pending_set),
    .CLEAR_n    (reg_wr_pending_clr),
    .Q          (reg_wr_pending)
);

ff_toggle_jk ff_toggle_rd (
    .CLK        (PHI_0),
    .RESET      (RESET),
    .SET_n      (reg_rd_pending_set),
    .CLEAR_n    (reg_rd_pending_clr),
    .Q          (reg_rd_pending)
);

ff_toggle_jk ff_toggle_kbd (
    .CLK        (PHI_0),
    .RESET      (RESET),
    .SET_n      (reg_kbd_pending_set),
    .CLEAR_n    (reg_kbd_pending_clr),
    .Q          (reg_kbd_pending)
);

always @(*) begin
    if (~C01X_N)                D7_status_line          = D7_c01x;
    else if (~C06X_N)           D7_status_line          = D7_c06x;
    else if (~DHIRES_state_n)   D7_status_line          = AN3;
    else                        D7_status_line          = 1'b0;
end

always @(*) begin
    reg_out_x           = reg_out_r;

    dbus_out_x          = dbus_out_r;
    dbus_en_int         = 1'b1;

    reg_wr_pending_set      = 1'b1;
    reg_rd_pending_clr      = 1'b1;
    reg_kbd_pending_clr     = 1'b1;

    if (~C0FX_N) begin
        if (ADDR[1:0] == 2'b01) begin
            dbus_en_int             = 1'b0;
            if (~CPU_RW) begin
                reg_out_x           = dbus_in;
                //$display("WR %02x", dbus_in);
                reg_wr_pending_set  = 1'b0;
            end else begin
                dbus_out_x          = { reg_wr_pending, 7'b0 };
            end
        end else if (ADDR[1:0] == 2'b10) begin
            if (CPU_RW) begin
                dbus_en_int         = 1'b0;
                dbus_out_x          = reg_rd_data;
                reg_rd_pending_clr  = 1'b0;
            end
        end else if (ADDR[1:0] == 2'b11) begin
            if (CPU_RW) begin
                dbus_en_int         = 1'b0;
                dbus_out_x          = { reg_rd_pending, 7'b0 };
            end
        end
    end else if(~KBDSEL_n) begin
        if (CPU_RW) begin
            dbus_en_int             = 1'b0;
            //$display("kdb %x", reg_kbd_data);
            dbus_out_x              = { reg_kbd_pending, reg_kbd_data };
        end
    end else begin  
        if (CPU_RW) begin
            if (D7_status_en) begin
                dbus_out_x          = { D7_status_line, 7'b0 };
                dbus_en_int         = 1'b0;
            end
        end else begin
            if(~C01X_N) begin
                reg_kbd_pending_clr = 1'b0;
            end
        end
    end
end

always @(posedge PHI_0) begin
    if (~RESET) begin
        dbus_out_r          <= 8'b0;

    end else begin
        dbus_out_r          <= dbus_out_x;
    end
end

always @(negedge PHI_0) begin
    if (~RESET) begin
        reg_out_r           <= 8'b0;

    end else begin
        reg_out_r           <= reg_out_x;
    end
end

endmodule
