module MMU_addr16 (
    input           PHI_0,
    input           RESET,
    input           CPU_RW,

    input           D_FXXX_INT,
    input           M_0XXX_BXXX_INT, 
    input           M_00_01XX_INT,  
    input           M_04_07FF_INT,  
    input           M_2_3FFF_INT,   
    input           exp16_ram_en,

    input           SW_RAMRD,
    input           SW_RAMWR,
    input           SW_80STORE,
    input           SW_HIRES,
    input           SW_PAGE2,
    input           SW_ALTZP,

    output          addr_16_out
);

wire   aux_rd_wr        = CPU_RW ? SW_RAMRD : SW_RAMWR;
//wire   addr_16_lores    = SW_80STORE ? SW_PAGE2 : aux_rd_wr;
//wire   addr_16_hires    = (SW_80STORE & SW_HIRES) ? SW_PAGE2 : aux_rd_wr;
//wire   addr_ram_normal  = M_0XXX_BXXX_INT & ~(M_00_01XX_INT | M_04_07FF_INT | M_2_3FFF_INT);

reg addr_16_reg_r;

always @(posedge PHI_0) begin
    if (~RESET) begin
        addr_16_reg_r           <= 1'b0;

    end else begin
        if (M_0XXX_BXXX_INT) begin
            if (M_00_01XX_INT) begin
                addr_16_reg_r       <= SW_ALTZP;

            end else if (M_04_07FF_INT) begin
                if (SW_80STORE) begin
                    addr_16_reg_r   <= SW_PAGE2;
                end else begin
                    addr_16_reg_r   <= aux_rd_wr;
                end

            end else if (M_2_3FFF_INT) begin
                if (SW_80STORE & SW_HIRES) begin
                    addr_16_reg_r   <= SW_PAGE2;
                end else begin
                    addr_16_reg_r   <= aux_rd_wr;
                end

            end else begin
                addr_16_reg_r       <= aux_rd_wr;
            end

        end else if (D_FXXX_INT & exp16_ram_en) begin
            addr_16_reg_r           <= SW_ALTZP;

        end else begin
            addr_16_reg_r           <= 1'b0;
        end
    end
end

//wire   addr_16          = (M_00_01XX_INT & SW_ALTZP) |
//                          (M_04_07FF_INT & addr_16_lores) |
//                          (M_2_3FFF_INT & addr_16_hires) |
//                          (addr_ram_normal & aux_rd_wr) |
//                          (D_FXXX_INT & SW_ALTZP & exp16_ram_en);

// The main address logic
assign addr_16_out      = addr_16_reg_r;

endmodule
