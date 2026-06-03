/* aarch64 (arm64) va_arg helpers for the mes libc, compiled by tcc_s.
 *
 * tcc's native __va_arg builtin takes the *address* of its va_list operand
 * (gen_va_arg does gaddrof()).  That only works when the operand is the va_list
 * storage itself (a local "va_list" array).  The mes libc forwards a va_list to
 * another function (printf -> vprintf -> vfprintf): because va_list is declared
 * as an array (va_list[1], matching AAPCS64/upstream tcc), the callee's
 * parameter is a *pointer* to the caller's struct, and __va_arg then takes the
 * address of that pointer — reading the wrong arguments.
 *
 * We therefore keep tcc's native __va_start (only ever used in the frame that
 * owns the varargs, where the operand is a genuine local array, so gaddrof is
 * correct) but implement va_arg in C.  These helpers take the va_list, which —
 * being an array type — decays to "__va_list *" uniformly whether it is a local
 * array or a forwarded parameter.  So a single pointer-based implementation is
 * correct in every frame.
 *
 * The fetch logic mirrors arm64-gen.c's gen_va_arg: general-purpose arguments
 * walk __gr_offs/__gr_top (overflowing to __stack); floating-point arguments
 * walk __vr_offs/__vr_top.  mes only fetches <=8-byte GP values (int, long,
 * pointers) and 8-byte doubles, so 8-byte GP and 16-byte VR steps suffice. */

#include <stdarg.h>

/* General-purpose argument: int / long / pointer (slot size always 8). */
void *
__mes_va_arg_gp (va_list ap)
{
  int offs = ap->__gr_offs;
  void *addr;
  if (offs + 8 <= 0)
    {
      addr = (char *) ap->__gr_top + offs;
      ap->__gr_offs = offs + 8;
    }
  else
    {
      addr = ap->__stack;
      ap->__stack = (char *) ap->__stack + 8;
    }
  return addr;
}

/* Floating-point argument: double (VR slot step 16, stack step 8). */
void *
__mes_va_arg_fp (va_list ap)
{
  int offs = ap->__vr_offs;
  void *addr;
  if (offs + 16 <= 0)
    {
      addr = (char *) ap->__vr_top + offs;
      ap->__vr_offs = offs + 16;
    }
  else
    {
      addr = ap->__stack;
      ap->__stack = (char *) ap->__stack + 8;
    }
  return addr;
}
