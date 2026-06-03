/* aarch64 (arm64) C runtime startup for the mes libc, compiled by tcc_s.
 *
 * tcc 0.9.26 has no arm64 assembler, so the entry glue is emitted as raw
 * instruction words via __asm__(".int 0x..") (validated with llvm-mc), while
 * __init_io() and main() are reached through ordinary C calls so that tcc
 * generates the bl relocations.  This mirrors mes' 32-bit arm crt1.c
 * (lib/linux/arm-mes-gcc/crt1.c, __TINYC__ branch).
 *
 * tcc's arm64 function prologue is "stp x29,x30,[sp,#-224]!; mov x29,sp", so at
 * _start entry the kernel-provided stack (argc, argv[], NULL, envp[], NULL)
 * begins at x29+224: argc = [x29,#224], &argv[0] = x29+232.
 */

#include <mes/lib-mini.h>
/* K&R declaration (unspecified args) so the argument-less call below — which
 * relies on argc/argv/envp already sitting in x0/x1/x2 — passes tcc's arity
 * check.  The x86_64 crt1.c sidesteps this by calling main from inside asm. */
int main ();

void
_start ()
{
  __asm__ (".int 0xf94073a0"); /* ldr x0, [x29, #224]  ; x0 = argc            */
  __asm__ (".int 0x9103a3a1"); /* add x1, x29, #232     ; x1 = &argv[0]        */
  __asm__ (".int 0x91000402"); /* add x2, x0, #1                              */
  __asm__ (".int 0xd37df042"); /* lsl x2, x2, #3        ; (argc+1) * 8         */
  __asm__ (".int 0x8b010042"); /* add x2, x2, x1        ; x2 = envp            */
  __init_io ();
  /* __init_io clobbers x0-x2; recompute argc/argv/envp for main. */
  __asm__ (".int 0xf94073a0"); /* ldr x0, [x29, #224]                         */
  __asm__ (".int 0x9103a3a1"); /* add x1, x29, #232                           */
  __asm__ (".int 0x91000402"); /* add x2, x0, #1                              */
  __asm__ (".int 0xd37df042"); /* lsl x2, x2, #3                              */
  __asm__ (".int 0x8b010042"); /* add x2, x2, x1                              */
  main ();
  /* exit (main_result): main's return value is already in x0. */
  __asm__ (".int 0xd2800ba8"); /* movz x8, #93          ; __NR_exit           */
  __asm__ (".int 0xd4000001"); /* svc #0                                      */
}
