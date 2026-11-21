module MMU_slot_ctrl (
    input          PHI_0,
    input          RESET,
    input          IOSEL_INT,
    input          CFFF_INT,
    input          SW_SLOTXROM,
    input          SW_SLOT3ROM,
    input   [2:0]  ADDR_10_8,

    output [2:0]   slot_index,
    output         rom_fw_enable
);

reg [2:0] slot_index_r;
reg       release_slot_r;

wire [2:0] addressed_slot = IOSEL_INT ? ADDR_10_8 : slot_index_r;

assign slot_index    = slot_index_r;
assign rom_fw_enable = SW_SLOTXROM | (~SW_SLOT3ROM & (addressed_slot == 3'd3));

always @(posedge PHI_0) begin
    if (~RESET) begin
        slot_index_r   <= 3'd0;
        release_slot_r <= 1'b0;
    end else begin
        // Keep the selected slot through the CFFF transfer. A new IOSEL
        // takes effect immediately; otherwise the following cycle releases it.
        release_slot_r <= CFFF_INT;
        if (IOSEL_INT) begin
            slot_index_r <= ADDR_10_8;
        end else if (release_slot_r) begin
            slot_index_r <= 3'd0;
        end
    end
end

endmodule
