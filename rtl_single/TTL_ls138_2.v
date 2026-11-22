module TTL_ls138_2 (
    input  [3:0] ADDR,
    input  G1, G2A_N, G2B_N,
    output Y0, Y1, Y2, Y3, Y4, Y5, Y6, Y7,
    output Y8, Y9, Y10, Y11, Y12, Y13, Y14, Y15
);

    wire G_N;
    wire NOT_A, NOT_B, NOT_C, NOT_D;

    assign G_N   = (~G1) | G2A_N | G2B_N;

    assign NOT_A = ~ADDR[0];
    assign NOT_B = ~ADDR[1];
    assign NOT_C = ~ADDR[2];
    assign NOT_D = ~ADDR[3];

    assign Y0 = G_N |  (ADDR[3] | ADDR[2] | ADDR[1] | ADDR[0]);
    assign Y1 = G_N |  (ADDR[3] | ADDR[2] | ADDR[1] | NOT_A);
    assign Y2 = G_N |  (ADDR[3] | ADDR[2] | NOT_B   | ADDR[0]);
    assign Y3 = G_N |  (ADDR[3] | ADDR[2] | NOT_B   | NOT_A);
    assign Y4 = G_N |  (ADDR[3] | NOT_C   | ADDR[1] | ADDR[0]);
    assign Y5 = G_N |  (ADDR[3] | NOT_C   | ADDR[1] | NOT_A);
    assign Y6 = G_N |  (ADDR[3] | NOT_C   | NOT_B   | ADDR[0]);
    assign Y7 = G_N |  (ADDR[3] | NOT_C   | NOT_B   | NOT_A);
    assign Y8 = G_N |  (NOT_D   | ADDR[2] | ADDR[1] | ADDR[0]);
    assign Y9 = G_N |  (NOT_D   | ADDR[2] | ADDR[1] | NOT_A);
    assign Y10 = G_N | (NOT_D   | ADDR[2] | NOT_B   | ADDR[0]);
    assign Y11 = G_N | (NOT_D   | ADDR[2] | NOT_B   | NOT_A);
    assign Y12 = G_N | (NOT_D   | NOT_C   | ADDR[1] | ADDR[0]);
    assign Y13 = G_N | (NOT_D   | NOT_C   | ADDR[1] | NOT_A);
    assign Y14 = G_N | (NOT_D   | NOT_C   | NOT_B   | ADDR[0]);
    assign Y15 = G_N | (NOT_D   | NOT_C   | NOT_B   | NOT_A);

endmodule
