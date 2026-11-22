module io_common (
    input           RESET,
    input           PHI_0,
    input           CPU_RW,
    input           C0XX_N,
    input [7:0]     ADDR,

    output          DHIRES_state_n,
    output          PDL_STROBE_n,
    output          KBDSEL_n,
    output          DEVSEL_n,
    output          C01X_N,
    output          C06X_N,
    output          C0BX_N,
    output          C0EX_N,
    output          C0FX_N,

    output          D7_c01x,
    output          D7_c06x,

    output          CAS_OUT,
    output          SPK_OUT,

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
    output          AN3,

    input           KBD_PENDING_IN,
    input           VIDEO_VLB,
    input           CAS_IN,
    input           SW0,
    input           SW1,

    input           PDL0,
    input           PDL1
);

assign      DHIRES_state_n   = DHIRES_state_wire;
assign      PDL_STROBE_n     = PDL_STROBE_wire;

assign      KBDSEL_n         = C00X | ~CPU_RW;
assign      DEVSEL_n         = DEVSEL_N_int;

assign      C01X_N           = C01X;
assign      C06X_N           = C06X | ~CPU_RW;
assign      C0BX_N           = C0BX;
assign      C0EX_N           = C0EX;
assign      C0FX_N           = C0FX;

assign      D7_c01x          = D7_c01x_int;
assign      D7_c06x          = D7_c06x_int;

assign      CAS_OUT          = casout_internal;
assign      SPK_OUT          = spk_internal;

wire        C02X, C03X;

wire        C00X, C01X, C04X_unused, C05X, C06X, C07X;
wire        C08X, C09X, C0AX, C0BX, C0CX, C0DX, C0EX, C0FX;

wire        DHIRES_state_wire = C07X | ~(ADDR[3] & ADDR[2] & ADDR[1] & ADDR[0]);
wire        PDL_STROBE_wire   = C07X |  (ADDR[3] | ADDR[2] | ADDR[1] | ADDR[0]);

wire        DEVSEL_N_int      = C09X & C0AX & C0CX & C0DX; //  | C0EX;

wire        casout_internal;
wire        spk_internal;

wire        D7_c01x_int;
wire        D7_c06x_int;

wire        SW_80COL;
wire        SW_CHARSET;
wire        SW_TEXT;
wire        SW_MIXED;


ff_toggle_d casout_toggle (
    .PHI_0  (PHI_0),
    .CS     (C02X),
    .RESET  (RESET), 
    .Q      (casout_internal)
);

ff_toggle_d spk_toggle (
    .PHI_0  (PHI_0),
    .CS     (C03X),
    .RESET  (RESET), 
    .Q      (spk_internal)
);

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
    .Q6     (SW_80COL),
    .Q7     (SW_CHARSET)
);

TTL_ls251_2 my_ls251_c01x (
    .D0     (KBD_PENDING_IN),
    .D1     (EXP16_BANK),
    .D2     (EXP16_RAM),
    .D3     (SW_RAMRD),
    .D4     (SW_RAMWR),
    .D5     (SW_SLOTXROM),
    .D6     (SW_ALTZP),
    .D7     (SW_SLOT3ROM),
    .D8     (SW_80STORE),
    .D9     (VIDEO_VLB),
    .D10    (SW_TEXT),
    .D11    (SW_MIXED),
    .D12    (SW_PAGE2),
    .D13    (SW_HIRES),
    .D14    (SW_CHARSET),
    .D15    (SW_80COL),

    .ADDR   (ADDR[3:0]),
    .ENABLE (C01X),

    .Q      (D7_c01x_int)
);

TTL_ls259 my_ls259_c05x (
    .C_N    (RESET),
    .E_N    (C05X | PHI_0),
    .D      (ADDR[0]),
    .ADDR   (ADDR[3:1]),

    // Output registers
    .Q0     (SW_TEXT),
    .Q1     (SW_MIXED),
    .Q2     (SW_PAGE2),
    .Q3     (SW_HIRES),
    .Q4     (AN0),
    .Q5     (AN1),
    .Q6     (AN2),
    .Q7     (AN3)
);

TTL_ls251 my_ls251_c06x (
    .D0     (CAS_IN),
    .D1     (SW0),
    .D2     (SW1),
    .D3     (1'b0), //SW2),
    .D4     (PDL0),
    .D5     (PDL1),
    .D6     (1'b0), //PDL2),
    .D7     (1'b0), //PDL3),

    .ADDR   (ADDR[2:0]),
    .ENABLE (C06X),

    .Q      (D7_c06x_int)
);

// Example instantiation of mem_exp16 module
mem_exp16 u_io_exp16 (
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
    .Y1     (C01X),
    .Y2     (C02X),
    .Y3     (C03X),
    .Y4     (C04X_unused),
    .Y5     (C05X),
    .Y6     (C06X),
    .Y7     (C07X),
    .Y8     (C08X),
    .Y9     (C09X),
    .Y10    (C0AX),
    .Y11    (C0BX),
    .Y12    (C0CX),
    .Y13    (C0DX),
    .Y14    (C0EX),
    .Y15    (C0FX)
);


endmodule
