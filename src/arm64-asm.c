/*************************************************************/
/*
 *  AArch64 dummy assembler for TCC
 *
 *  tcc 0.9.26 ships no arm64 assembler, so any inline asm() errors out with
 *  "inline asm() not supported".  This minimal backend defines CONFIG_TCC_ASM
 *  and supports only the data-emitting directives handled generically by
 *  tccasm.c (.byte/.word/.int/.long, .align, .skip, labels, .global).  Real
 *  instruction mnemonics are rejected.  That is enough for the mes libc
 *  primitives, which emit raw aarch64 instruction words via __asm__(".int 0x..")
 *  and reach symbols (main, __init_io, ...) through ordinary C calls — exactly
 *  the approach mes already uses for 32-bit arm (lib/linux/arm-mes-gcc/*.c).
 */

#ifdef TARGET_DEFS_ONLY

#define CONFIG_TCC_ASM
#define NB_ASM_REGS 32

ST_FUNC void g(int c);
ST_FUNC void gen_le16(int c);
ST_FUNC void gen_le32(int c);

/*************************************************************/
#else
/*************************************************************/

#include "tcc.h"

static void asm_error(void)
{
    tcc_error("AArch64 asm not implemented (only .int/.word data directives are supported).");
}

/* XXX: make it faster ? */
ST_FUNC void g(int c)
{
    int ind1;
    if (nocode_wanted)
        return;
    ind1 = ind + 1;
    if (ind1 > cur_text_section->data_allocated)
        section_realloc(cur_text_section, ind1);
    cur_text_section->data[ind] = c;
    ind = ind1;
}

ST_FUNC void gen_le16 (int i)
{
    g(i);
    g(i>>8);
}

ST_FUNC void gen_le32 (int i)
{
    gen_le16(i);
    gen_le16(i>>16);
}

ST_FUNC void gen_expr32(ExprValue *pe)
{
    gen_le32(pe->v);
}

ST_FUNC void asm_opcode(TCCState *s1, int opcode)
{
    asm_error();
}

ST_FUNC void subst_asm_operand(CString *add_str, SValue *sv, int modifier)
{
    asm_error();
}

/* generate prolog and epilog code for asm statement */
ST_FUNC void asm_gen_code(ASMOperand *operands, int nb_operands,
                         int nb_outputs, int is_output,
                         uint8_t *clobber_regs,
                         int out_reg)
{
}

ST_FUNC void asm_compute_constraints(ASMOperand *operands,
                                    int nb_operands, int nb_outputs,
                                    const uint8_t *clobber_regs,
                                    int *pout_reg)
{
}

ST_FUNC void asm_clobber(uint8_t *clobber_regs, const char *str)
{
    asm_error();
}

ST_FUNC int asm_parse_regvar (int t)
{
    asm_error();
    return -1;
}

/*************************************************************/
#endif /* ndef TARGET_DEFS_ONLY */
