module MMU_slot_ctrl (
    input          PHI_0,
    input          RESET,
    input          IOSEL_INT,
    input          IOSTB_INT,
    input          CFFF_INT,
    input          SW_SLOTXROM,
    input          SW_SLOT3ROM,
    input   [2:0]  ADDR_10_8,

    output [2:0]   slot_index,
    output         rom_fw_enable
);

assign rom_fw_enable    = rom_fw_enable_r;
assign slot_index       = slot_index_r;

wire slot_3_sel         = (~ADDR_10_8[2] & ADDR_10_8[1] & ADDR_10_8[0]);

//wire slot_0_index       = (~slot_index_r[2] & ~slot_index_r[1] & ~slot_index_r[0]);
wire slot_3_index       = (~slot_index_r[2] &  slot_index_r[1] &  slot_index_r[0]);

reg [2:0]   slot_index_r,       slot_index_x;
reg         rom_fw_enable_r,    rom_fw_enable_x;

always @(*) begin
    slot_index_x                = slot_index_r;
    rom_fw_enable_x             = rom_fw_enable_r;

    if (IOSEL_INT) begin
        slot_index_x            = ADDR_10_8;
        rom_fw_enable_x         = (SW_SLOTXROM || (~SW_SLOT3ROM & slot_3_sel)); 

    end else if (CFFF_INT) begin
        slot_index_x            = 3'd0;

    end else if (IOSTB_INT) begin
        rom_fw_enable_x         = (SW_SLOTXROM || (~SW_SLOT3ROM & slot_3_index));
    end
end

always @(posedge PHI_0) begin
    if (~RESET) begin
        slot_index_r        <= 3'd0;
        rom_fw_enable_r     <= 1'b0;
    end else begin
        slot_index_r        <= slot_index_x;
        rom_fw_enable_r     <= rom_fw_enable_x;

        //if( CFFF_INT ) begin
        //    $display("slot = %d", slot_index_x);
        //end
    end
end

endmodule
