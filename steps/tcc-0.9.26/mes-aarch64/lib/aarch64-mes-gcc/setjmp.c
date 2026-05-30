/* aarch64 (arm64) setjmp/longjmp for the mes libc, compiled by tcc_s.
 *
 * tcc 0.9.26 has no arm64 assembler, so each instruction is a raw word via
 * __asm__(".int 0x..") (encodings verified with llvm-mc --triple=aarch64).
 * This mirrors the 32-bit arm __TINYC__ path (lib/arm-mes-gcc/setjmp.c).
 *
 * Layout, kept in sync with the __aarch64__ branch of include/setjmp.h:
 *   __registers[10] = x19..x28   (offsets 0..72)
 *   __fp            = x29        (offset 80)
 *   __lr            = x30        (offset 88)
 *   __sp            = sp         (offset 96)
 *
 * At entry x0 = env (the jmp_buf pointer); tcc keeps the incoming argument in
 * x0 across the prologue, and we never reference it from C, so the raw words
 * may use x0 directly.
 *
 * Capturing the CALLER's frame (not setjmp's own) is what makes non-local
 * jumps correct.  tcc's prologue for these functions is
 *     stp x29, x30, [sp, #-FS]!   ;  ... ; mov x29, sp
 * so once x29 is the frame base:
 *     caller fp  = [x29]      (the old x29 the prologue just pushed)
 *     caller lr  = x30        (still holds the return address into the caller)
 *     caller sp  = x29 + FS   (sp as it was at the bl setjmp site)
 * FS is the tcc-chosen frame size for THIS function body (currently 224 — see
 * the `add x1, x29, #224` below); it is asserted by re-disassembling after a
 * build.  setjmp returns 0 via a normal C `return`, so tcc emits its real
 * epilogue (mov sp,x29; ldp x29,x30,[sp],#FS; ret) and the caller's sp/fp/lr
 * are restored correctly.  longjmp restores the saved caller frame and `ret`s
 * straight into the caller (it must bypass its own epilogue, since it is
 * jumping, not returning).
 */

#include <setjmp.h>
#include <stdlib.h>

int
__attribute__ ((noinline))
setjmp (jmp_buf env)
{
  __asm__ (".int 0xa9005013"); /* stp x19, x20, [x0]      */
  __asm__ (".int 0xa9015815"); /* stp x21, x22, [x0, #16] */
  __asm__ (".int 0xa9026017"); /* stp x23, x24, [x0, #32] */
  __asm__ (".int 0xa9036819"); /* stp x25, x26, [x0, #48] */
  __asm__ (".int 0xa904701b"); /* stp x27, x28, [x0, #64] */
  __asm__ (".int 0xf94003a1"); /* ldr x1, [x29]           ; caller fp */
  __asm__ (".int 0xf9002801"); /* str x1, [x0, #80]       */
  __asm__ (".int 0xf9002c1e"); /* str x30, [x0, #88]      ; caller lr */
  __asm__ (".int 0x910383a1"); /* add x1, x29, #224       ; caller sp = x29+FS */
  __asm__ (".int 0xf9003001"); /* str x1, [x0, #96]       */
  return 0;                    /* tcc epilogue restores caller sp/fp/lr */
}

void
__attribute__ ((noinline))
longjmp (jmp_buf env, int val)
{
  __asm__ (".int 0xa9405013"); /* ldp x19, x20, [x0]      */
  __asm__ (".int 0xa9415815"); /* ldp x21, x22, [x0, #16] */
  __asm__ (".int 0xa9426017"); /* ldp x23, x24, [x0, #32] */
  __asm__ (".int 0xa9436819"); /* ldp x25, x26, [x0, #48] */
  __asm__ (".int 0xa944701b"); /* ldp x27, x28, [x0, #64] */
  __asm__ (".int 0xa945781d"); /* ldp x29, x30, [x0, #80] */
  __asm__ (".int 0xf9403002"); /* ldr x2, [x0, #96]       */
  __asm__ (".int 0x9100005f"); /* mov sp, x2              */
  __asm__ (".int 0xf100003f"); /* cmp x1, #0              */
  __asm__ (".int 0x9a9f1420"); /* csinc x0, x1, xzr, ne   ; 0->1 */
  __asm__ (".int 0xd65f03c0"); /* ret                     */
  /* not reached */
}
