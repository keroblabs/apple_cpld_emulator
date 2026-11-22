module memory_map(
    input  [15:0] ADDR,
    output        D_FXXX_INT,
    output        DXXX_INT,
    output        IOSTB_INT,
    output        IOSEL_INT,
    output        CFFF_INT,
    output        C0XX_INT,

    output        M_0XXX_BXXX_INT, 
    output        M_00_01XX_INT,  
    output        M_04_07FF_INT,  
    output        M_2_3FFF_INT        
);

wire CXXX_FXXX_INT_wire = ADDR[15] & ADDR[14];
wire FXXX_INT_wire      = CXXX_FXXX_INT_wire &  ADDR[13] &  ADDR[12];
wire EXXX_INT_wire      = CXXX_FXXX_INT_wire &  ADDR[13] & ~ADDR[12];
wire DXXX_INT_wire      = CXXX_FXXX_INT_wire & ~ADDR[13] &  ADDR[12];
wire CXXX_INT_wire      = CXXX_FXXX_INT_wire & ~ADDR[13] & ~ADDR[12];
wire IOSTB_INT_wire     = CXXX_INT_wire & ADDR[11];
wire IOSEL_INT_wire     = CXXX_INT_wire & ~ADDR[11] & (ADDR[10] | ADDR[9] | ADDR[8]);
wire C0XX_INT_wire      = CXXX_INT_wire & ~ADDR[11] & ~ADDR[10] & ~ADDR[9] & ~ADDR[8];
wire CFFF_INT_wire      = IOSTB_INT_wire
                            & ADDR[10] & ADDR[9] & ADDR[8] & ADDR[7] & ADDR[6]
                            & ADDR[5] & ADDR[4] & ADDR[3] & ADDR[2] & ADDR[1] & ADDR[0];

wire E_FXXX_INT_wire    = EXXX_INT_wire | FXXX_INT_wire;
wire D_FXXX_INT_wire    = DXXX_INT_wire | E_FXXX_INT_wire;

//always @(*) begin
//    $display("IOSEL_INT_wire = %d", IOSEL_INT_wire);
//end

wire M_0XXX_BXXX_INT_wire   = ~CXXX_FXXX_INT_wire;
wire M_00_01XX_INT_wire     = ~(ADDR[15] | ADDR[14] | ADDR[13] | ADDR[12] | ADDR[11] | ADDR[10] | ADDR[9]);
wire M_04_07FF_INT_wire     = ~(ADDR[15] | ADDR[14] | ADDR[13] | ADDR[12] | ADDR[11]) & ADDR[10];
wire M_2_3FFF_INT_wire      = ~(ADDR[15] | ADDR[14]) & ADDR[13];

// Assign outputs
assign D_FXXX_INT       = D_FXXX_INT_wire;
assign DXXX_INT         = DXXX_INT_wire;
assign CFFF_INT         = CFFF_INT_wire;
assign C0XX_INT         = C0XX_INT_wire;

assign IOSTB_INT        = IOSTB_INT_wire;
assign IOSEL_INT        = IOSEL_INT_wire;

assign M_0XXX_BXXX_INT  = M_0XXX_BXXX_INT_wire;
assign M_00_01XX_INT    = M_00_01XX_INT_wire;
assign M_04_07FF_INT    = M_04_07FF_INT_wire;
assign M_2_3FFF_INT     = M_2_3FFF_INT_wire;

endmodule
