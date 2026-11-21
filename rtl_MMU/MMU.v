`define DONT_ADD_UART

module MMU (
    input [15:0]    ADDR,

`ifdef ADD_UART
    inout  [7:0]    DATA,
    output          DATA_EN_N,

    input           uart_clock,
`endif

    input           CPU_RW,
    input           PHI_0,
    input           RESET,
    input           DMA_N,
    input           MODE_SEL,

`ifdef SPK_IN_MMU
    output          CAS_OUT,
    output          SPK_OUT,
`endif

    output          PHIO_0,
    output          BUS_N,

`ifdef FULL_MMU_ADDR
    output [16:0]   MEM_ADDR_OUT,
`else
    output [4:0]    MEM_ADDR_OUT,
`endif
    output          RAM_EN_N,
    output          ROM_EN_N,
    output          BUS_RD_N,
    output          BUS_WR_N,

`ifdef ADD_UART
    output          uart_tx_clock_o,
    output          uart_rx_clock_o,
    input           uart_rx_pin_i,
    output          uart_tx_pin_o,
`endif

    output          C0XX_N_OUT,
    output          IOSEL_OUT_N,
    output          C6XX_N,
    output          IOSTB_N

    //output          AN0,
    //output          AN1,
    //output          AN2,
    //output          AN3

);

assign        PHIO_0        = PHI_0 & DMA_N;
assign        BUS_N         = ~DMA_N;

`ifdef ADD_UART
assign          DATA          = CPU_RW ? dbus_out : 8'bzzzzzzzz;
assign          dbus_in       = DATA; //  CPU_RW ? 8'd0 : DATA;
assign          DATA_EN_N     = dbus_en;

wire  [7:0]     dbus_in;
wire  [7:0]     dbus_out;
wire            dbus_en;
`endif

wire          EXP16_BANK;
wire          EXP16_RAM;
wire          EXP16_WRITE;

wire          SW_PAGE2;
wire          SW_HIRES;
wire          SW_RAMRD;
wire          SW_RAMWR;
wire          SW_ALTZP;
wire          SW_SLOTXROM;
wire          SW_SLOT3ROM;
wire          SW_80STORE;

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
    .C0XX_N          (C0XX_N_OUT),
    .C6XX_N          (C6XX_N),

    .MEM_ADDR_OUT    (MEM_ADDR_OUT)
);

MMU_io u_IOU_common (
    // Inputs
    .RESET             (RESET),
    .PHI_0             (PHI_0),
    .CPU_RW            (CPU_RW),
    .C0XX_N            (C0XX_N_OUT),
    .ADDR              (ADDR[7:0]),

`ifdef SPK_IN_MMU
    .CAS_OUT         (CAS_OUT),
    .SPK_OUT         (SPK_OUT),
`endif 

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
    //.AN0             (AN0),
    //.AN1             (AN1),
    //.AN2             (AN2),
    //.AN3             (AN3)
);

`ifdef ADD_UART
wire uart_cs        = ~C0XX_N_OUT & (ADDR[7:4] == 4'b1010);
assign dbus_en      = uart_cs;

uart_block u_uart_block(
    .clk_i              (uart_clock),
    .reset_i            (RESET),

    .addr_i             (ADDR[1:0]),
    .wdata_i            (dbus_in),
    .rdata_o            (dbus_out),
    .cs_i               (uart_cs),
    .rw_i               (CPU_RW),

    .uart_tx_clock_o    (uart_tx_clock_o),
    .uart_rx_clock_o    (uart_rx_clock_o),
    .uart_rx_pin_i      (uart_rx_pin_i),
    .uart_tx_pin_o      (uart_tx_pin_o)    
);
`endif 

endmodule
