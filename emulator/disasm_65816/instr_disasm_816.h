#ifndef instr_disasm_816_h
#define instr_disasm_816_h

INLINE static void op_adc(uint32_t ea)
{
    strcpy(str_opcode, "ADC");
    disasm_comment |= SHOW_REG_ACC;
}

INLINE static void op_and(uint32_t ea)
{
    strcpy(str_opcode, "AND");
    disasm_comment |= SHOW_REG_ACC;
}

INLINE static void op_asl(uint32_t ea)
{
    strcpy(str_opcode, "ASL");
    disasm_comment |= SHOW_MEM;
    comment_addr = ea;
}

INLINE static void op_asla(uint32_t ea)
{
    strcpy(str_opcode, "ASL");
    disasm_comment |= SHOW_REG_ACC;
}

INLINE static void op_bcc(uint32_t ea)
{
    strcpy(str_opcode, "BCC");
}

INLINE static void op_bcs(uint32_t ea)
{
    strcpy(str_opcode, "BCS");
}

INLINE static void op_beq(uint32_t ea)
{
    strcpy(str_opcode, "BEQ");
}

INLINE static void op_bit(uint32_t ea)
{
    strcpy(str_opcode, "BIT");
    disasm_comment |= SHOW_MEM | SHOW_FLAGS;
    comment_addr = ea;
}

INLINE static void op_biti(uint32_t ea)
{
    strcpy(str_opcode, "BIT");
    disasm_comment |= SHOW_MEM | SHOW_FLAGS;
    comment_addr = ea;
}

INLINE static void op_bmi(uint32_t ea)
{
    strcpy(str_opcode, "BMI");
}

INLINE static void op_bne(uint32_t ea)
{
    strcpy(str_opcode, "BNE");
}

INLINE static void op_bpl(uint32_t ea)
{
    strcpy(str_opcode, "BPL");
}

INLINE static void op_bra(uint32_t ea)
{
    strcpy(str_opcode, "BRA");
}

INLINE static void op_brk(uint32_t ea)
{
    strcpy(str_opcode, "BRK");
}

INLINE static void op_brl(uint32_t ea)
{
    strcpy(str_opcode, "BRL");
}

INLINE static void op_bvc(uint32_t ea)
{
    strcpy(str_opcode, "BVC");
}

INLINE static void op_bvs(uint32_t ea)
{
    strcpy(str_opcode, "BVS");
}

INLINE static void op_clc(uint32_t ea)
{
    strcpy(str_opcode, "CLC");
}

INLINE static void op_cld(uint32_t ea)
{
    strcpy(str_opcode, "CLD");
}

INLINE static void op_cli(uint32_t ea)
{
    strcpy(str_opcode, "CLI");
}

INLINE static void op_clv(uint32_t ea)
{
    strcpy(str_opcode, "CLD");
}

INLINE static void op_cmp(uint32_t ea)
{
    strcpy(str_opcode, "CMP");
    disasm_comment |= SHOW_REG_ACC;
}

INLINE static void op_cop(uint32_t ea)
{
    strcpy(str_opcode, "COP");
}

INLINE static void op_cpx(uint32_t ea)
{
    strcpy(str_opcode, "CPX");
    disasm_comment |= SHOW_REG_X;
}

INLINE static void op_cpy(uint32_t ea)
{
    strcpy(str_opcode, "CPY");
    disasm_comment |= SHOW_REG_Y;
}

INLINE static void op_dec(uint32_t ea)
{
    strcpy(str_opcode, "DEC");
    comment_addr = ea;
    disasm_comment |= SHOW_MEM;
}

INLINE static void op_deca(uint32_t ea)
{
    strcpy(str_opcode, "DEC");
    disasm_comment |= SHOW_REG_ACC;
}

INLINE static void op_dex(uint32_t ea)
{
    strcpy(str_opcode, "DEX");
    disasm_comment |= SHOW_REG_X;
}

INLINE static void op_dey(uint32_t ea)
{
    strcpy(str_opcode, "DEY");
    disasm_comment |= SHOW_REG_Y;
}

INLINE static void op_eor(uint32_t ea)
{
    strcpy(str_opcode, "EOR");
    disasm_comment |= SHOW_REG_ACC;
}

INLINE static void op_inc(uint32_t ea)
{
    strcpy(str_opcode, "INC");
    comment_addr = ea;
    disasm_comment |= SHOW_MEM;
}

INLINE static void op_inca(uint32_t ea)
{
    strcpy(str_opcode, "INC");
    disasm_comment |= SHOW_REG_ACC;
}

INLINE static void op_inx(uint32_t ea)
{
    strcpy(str_opcode, "INX");
    disasm_comment |= SHOW_REG_X;
}

INLINE static void op_iny(uint32_t ea)
{
    strcpy(str_opcode, "INY");
    disasm_comment |= SHOW_REG_Y;
}

INLINE static void op_jmp(uint32_t ea)
{
    strcpy(str_opcode, "JMP");
    comment_addr = ea;
    disasm_comment |= SHOW_JUMP;
}

INLINE static void op_jsl(uint32_t ea)
{
    strcpy(str_opcode, "JSL");
    disasm_comment |= SHOW_JUMP;
    comment_addr = ea;
}

INLINE static void op_jsr(uint32_t ea)
{
    strcpy(str_opcode, "JSR");
    disasm_comment |= SHOW_JUMP;
    comment_addr = ea;
}

INLINE static void op_lda(uint32_t ea)
{
    strcpy(str_opcode, "LDA");
    disasm_comment |= SHOW_MEM;
    comment_addr = ea;
}

INLINE static void op_ldx(uint32_t ea)
{
    strcpy(str_opcode, "LDX");
    disasm_comment |= SHOW_MEM;
    comment_addr = ea;
}

INLINE static void op_ldy(uint32_t ea)
{
    strcpy(str_opcode, "LDY");
    disasm_comment |= SHOW_MEM;
    comment_addr = ea;
}

INLINE static void op_lsr(uint32_t ea)
{
    strcpy(str_opcode, "LSR");
    disasm_comment |= SHOW_MEM;
    comment_addr = ea;
}

INLINE static void op_lsra(uint32_t ea)
{
    strcpy(str_opcode, "LSR");
    disasm_comment |= SHOW_REG_ACC;
}

INLINE static void op_mvn(uint32_t ea)
{
    strcpy(str_opcode, "MVN");
}

INLINE static void op_mvp(uint32_t ea)
{
    strcpy(str_opcode, "MVP");
}

INLINE static void op_nop(uint32_t ea)
{
    strcpy(str_opcode, "NOP");
}

INLINE static void op_ora(uint32_t ea)
{
    strcpy(str_opcode, "ORA");
    disasm_comment |= SHOW_REG_ACC;
}

INLINE static void op_pea(uint32_t ea)
{
    strcpy(str_opcode, "PEA");
}

INLINE static void op_pei(uint32_t ea)
{
    strcpy(str_opcode, "PEI");
}

INLINE static void op_per(uint32_t ea)
{
    strcpy(str_opcode, "PER");
}

INLINE static void op_pha(uint32_t ea)
{
    strcpy(str_opcode, "PHA");
    disasm_comment |= SHOW_REG_ACC;
}

INLINE static void op_phb(uint32_t ea)
{
    strcpy(str_opcode, "PHB");
}

INLINE static void op_phd(uint32_t ea)
{
    strcpy(str_opcode, "PHD");
}

INLINE static void op_phk(uint32_t ea)
{
    strcpy(str_opcode, "PHK");
}

INLINE static void op_php(uint32_t ea)
{
    strcpy(str_opcode, "PHP");
    disasm_comment |= SHOW_FLAGS;
}

INLINE static void op_phx(uint32_t ea)
{
    strcpy(str_opcode, "PHX");
    disasm_comment |= SHOW_REG_X;
}

INLINE static void op_phy(uint32_t ea)
{
    strcpy(str_opcode, "PHY");
    disasm_comment |= SHOW_REG_Y;
}

INLINE static void op_pla(uint32_t ea)
{
    strcpy(str_opcode, "PLA");
    disasm_comment |= SHOW_MEM; //comment_addr = STACK_ADDR;
    comment_addr = ea;
}

INLINE static void op_plb(uint32_t ea)
{
    strcpy(str_opcode, "PLB");
}

INLINE static void op_pld(uint32_t ea)
{
    strcpy(str_opcode, "PLD");
}

INLINE static void op_plk(uint32_t ea)
{
    strcpy(str_opcode, "PLK");
}

INLINE static void op_plp(uint32_t ea)
{
    strcpy(str_opcode, "PLP");
    disasm_comment |= SHOW_MEM; //comment_addr = STACK_ADDR;
    comment_addr = ea;
}

INLINE static void op_plx(uint32_t ea)
{
    strcpy(str_opcode, "PLX");
    disasm_comment |= SHOW_MEM; //comment_addr = STACK_ADDR;
    comment_addr = ea;
}

INLINE static void op_ply(uint32_t ea)
{
    strcpy(str_opcode, "PLY");
    disasm_comment |= SHOW_MEM; //comment_addr = STACK_ADDR;
    comment_addr = ea;
}

INLINE static void op_rep(uint32_t ea)
{
    strcpy(str_opcode, "REP");
}

INLINE static void op_rol(uint32_t ea)
{
    strcpy(str_opcode, "ROL");
    disasm_comment |= SHOW_MEM;
    comment_addr = ea;
}

INLINE static void op_rola(uint32_t ea)
{
    strcpy(str_opcode, "ROL");
    disasm_comment |= SHOW_REG_ACC;
}

INLINE static void op_ror(uint32_t ea)
{
    strcpy(str_opcode, "ROR");
    disasm_comment |= SHOW_MEM;
    comment_addr = ea;
}

INLINE static void op_rora(uint32_t ea)
{
    strcpy(str_opcode, "ROR");
    disasm_comment |= SHOW_REG_ACC;
}

INLINE static void op_rti(uint32_t ea)
{
    strcpy(str_opcode, "RTI");
}

INLINE static void op_rtl(uint32_t ea)
{
    strcpy(str_opcode, "RTL");
}

INLINE static void op_rts(uint32_t ea)
{
    strcpy(str_opcode, "RTS");
    disasm_comment |= SHOW_RETURN;
    comment_addr = ea;
}

INLINE static void op_sbc(uint32_t ea)
{
    strcpy(str_opcode, "SBC");
    disasm_comment |= SHOW_REG_ACC;
}

INLINE static void op_sec(uint32_t ea)
{
    strcpy(str_opcode, "SEC");
}

INLINE static void op_sed(uint32_t ea)
{
    strcpy(str_opcode, "SED");
}

INLINE static void op_sei(uint32_t ea)
{
    strcpy(str_opcode, "SEI");
}

INLINE static void op_sep(uint32_t ea)
{
    strcpy(str_opcode, "SEP");
}

INLINE static void op_sta(uint32_t ea)
{
    strcpy(str_opcode, "STA");
    disasm_comment |= SHOW_REG_ACC | SHOW_MEM;
    comment_addr = ea;
}

INLINE static void op_stp(uint32_t ea)
{
    strcpy(str_opcode, "STP");
}

INLINE static void op_stx(uint32_t ea)
{
    strcpy(str_opcode, "STX");
    disasm_comment |= SHOW_REG_X | SHOW_MEM;
    comment_addr = ea;
}

INLINE static void op_sty(uint32_t ea)
{
    strcpy(str_opcode, "STY");
    disasm_comment |= SHOW_REG_Y| SHOW_MEM;
    comment_addr = ea;
}

INLINE static void op_stz(uint32_t ea)
{
    strcpy(str_opcode, "STZ");
}

INLINE static void op_tax(uint32_t ea)
{
    strcpy(str_opcode, "TAX");
    disasm_comment |= SHOW_REG_X | SHOW_REG_ACC;
}

INLINE static void op_tay(uint32_t ea)
{
    strcpy(str_opcode, "TAY");
    disasm_comment |= SHOW_REG_Y | SHOW_REG_ACC;
}

INLINE static void op_tcd(uint32_t ea)
{
    strcpy(str_opcode, "TCD");
}

INLINE static void op_tdc(uint32_t ea)
{
    strcpy(str_opcode, "TDC");
}

INLINE static void op_tcs(uint32_t ea)
{
    strcpy(str_opcode, "TCS");
}

INLINE static void op_trb(uint32_t ea)
{
    strcpy(str_opcode, "TRB");
}

INLINE static void op_tsb(uint32_t ea)
{
    strcpy(str_opcode, "TSB");
}

INLINE static void op_tsc(uint32_t ea)
{
    strcpy(str_opcode, "TSC");
}

INLINE static void op_tsx(uint32_t ea)
{
    strcpy(str_opcode, "TSX");
}

INLINE static void op_txa(uint32_t ea)
{
    strcpy(str_opcode, "TXA");
    disasm_comment |= SHOW_REG_X | SHOW_REG_ACC;
}

INLINE static void op_txs(uint32_t ea)
{
    strcpy(str_opcode, "TXS");
    disasm_comment |= SHOW_REG_X;
}

INLINE static void op_txy(uint32_t ea)
{
    strcpy(str_opcode, "TXY");
    disasm_comment |= SHOW_REG_X | SHOW_REG_Y;
}

INLINE static void op_tya(uint32_t ea)
{
    strcpy(str_opcode, "TYA");
    disasm_comment |= SHOW_REG_Y | SHOW_REG_ACC;
}

INLINE static void op_tyx(uint32_t ea)
{
    strcpy(str_opcode, "TYX");
    disasm_comment |= SHOW_REG_X | SHOW_REG_Y;
}

INLINE static void op_wai(uint32_t ea)
{
    strcpy(str_opcode, "WAI");
}

INLINE static void op_wdm(uint32_t ea)
{
    strcpy(str_opcode, "WDM");
}

INLINE static void op_xba(uint32_t ea)
{
    strcpy(str_opcode, "XBA");
}

INLINE static void op_xce(uint32_t ea)
{
    strcpy(str_opcode, "XCE");
}

#endif /* instr_disasm_816_h */
