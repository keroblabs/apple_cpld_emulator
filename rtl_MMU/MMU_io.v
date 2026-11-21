module MMU_io (
    input           RESET,
    input           PHI_0,
    input           CPU_RW,
    input           C0XX_N,
    input [7:0]     ADDR,

`ifdef SPK_IN_MMU
    output          CAS_OUT,
    output          SPK_OUT,
`endif 

    output          EXP16_BANK,
    output          EXP16_RAM,
    output          EXP16_WRITE,

    output          SW_RAMRD,
    output          SW_RAMWR,
    output          SW_SLOTXROM,
    output          SW_SLOT3ROM,
    output          SW_ALTZP,
    output          SW_80STORE,

    output          SW_PAGE2,
    output          SW_HIRES,
    output          AN0,
    output          AN1,
    output          AN2,
    output          AN3

);

`ifdef SPK_IN_MMU
assign      CAS_OUT          = casout_internal;
assign      SPK_OUT          = spk_internal;
`endif 

`ifdef SPK_IN_MMU
wire        C02X, C03X;
`else
wire        C02X_unused, C03X_unused;
`endif
    
wire        C00X, C01X_unused, C04X_unused, C05X, C06X_unused, C07X_unused;
wire        C08X, C09X_unused, C0AX_unused, C0BX_unused, C0CX_unused, C0DX_unused, C0EX_unused, C0FX_unused;

`ifdef SPK_IN_MMU
wire        casout_internal;
wire        spk_internal;
`endif 

wire        SW_80COL_unused;
wire        SW_CHARSET_unused;
wire        SW_TEXT_unused;
wire        SW_MIXED_unused;

`ifdef SPK_IN_MMU
IOU_toggle_d casout_toggle (
    .PHI_0  (PHI_0),
    .CS     (C02X),
    .RESET  (RESET), 
    .Q      (casout_internal)
);

IOU_toggle_d spk_toggle (
    .PHI_0  (PHI_0),
    .CS     (C03X),
    .RESET  (RESET), 
    .Q      (spk_internal)
);
`endif


TTL_ls259 my_ls259_c00x (
    .C_N   (RESET),
    .E_N   (C00X | CPU_RW | PHI_0),
    .D     (ADDR[0]),
    .ADDR  (ADDR[3:1]),

    // Output registers
    .Q0     (SW_80STORE),
    .Q1     (SW_RAMRD),
    .Q2     (SW_RAMWR),
    .Q3     (SW_SLOTXROM),
    .Q4     (SW_ALTZP),
    .Q5     (SW_SLOT3ROM),
    .Q6     (SW_80COL_unused),
    .Q7     (SW_CHARSET_unused)
);

TTL_ls259 my_ls259_c05x (
    .C_N    (RESET),
    .E_N    (C05X | PHI_0),
    .D      (ADDR[0]),
    .ADDR   (ADDR[3:1]),

    // Output registers
    .Q0     (SW_TEXT_unused),
    .Q1     (SW_MIXED_unused),
    .Q2     (SW_PAGE2),
    .Q3     (SW_HIRES),
    .Q4     (AN0),
    .Q5     (AN1),
    .Q6     (AN2),
    .Q7     (AN3)
);

IOU_exp16 u_IOU_exp16 (
    .C08X       (C08X),
    .PHI_0      (PHI_0),
    .RESET      (RESET),
    .ADDR       (ADDR[3:0]),

    .EXP16_BANK (EXP16_BANK),
    .EXP16_RAM  (EXP16_RAM),
    .EXP16_WRITE(EXP16_WRITE)
);

TTL_ls138_2 my_ls138_internal_io (
    .ADDR   (ADDR[7:4]),
    .G1     (1'b1),
    .G2A_N  (1'b0),
    .G2B_N  (C0XX_N),

    // Outputs
    .Y0     (C00X),
    .Y1     (C01X_unused),
`ifdef SPK_IN_MMU
    .Y2     (C02X),
    .Y3     (C03X),
`else
    .Y2     (C02X_unused),
    .Y3     (C03X_unused),
`endif
    .Y4     (C04X_unused),
    .Y5     (C05X),
    .Y6     (C06X_unused),
    .Y7     (C07X_unused),
    .Y8     (C08X),
    .Y9     (C09X_unused),
    .Y10    (C0AX_unused),
    .Y11    (C0BX_unused),
    .Y12    (C0CX_unused),
    .Y13    (C0DX_unused),
    .Y14    (C0EX_unused),
    .Y15    (C0FX_unused)
);

endmodule
