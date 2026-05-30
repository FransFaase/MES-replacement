/* aarch64 _exit for the mes libc, compiled by tcc_s (raw .int, no asm support).
 * __NR_exit = 93; the exit code is already in x0 at entry. */

#include <mes/lib-mini.h>

void
_exit (int code)
{
  __asm__ (".int 0xd2800ba8"); /* movz x8, #93   ; __NR_exit */
  __asm__ (".int 0xd4000001"); /* svc #0                     */
  /* not reached */
  _exit (0);
}
