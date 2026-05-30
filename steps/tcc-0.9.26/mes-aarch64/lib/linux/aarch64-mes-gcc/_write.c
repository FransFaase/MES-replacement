/* aarch64 _write for the mes libc, compiled by tcc_s (raw .int, no asm support).
 * __NR_write = 64.  At entry x0=filedes, x1=buffer, x2=size are already in the
 * syscall argument registers, so we only set x8 and "svc #0"; the result in x0
 * flows out as the return value (tcc's epilogue preserves x0). */

#include <mes/lib-mini.h>

ssize_t
_write (int filedes, void const *buffer, size_t size)
{
  __asm__ (".int 0xd2800808"); /* movz x8, #64   ; __NR_write */
  __asm__ (".int 0xd4000001"); /* svc #0                      */
}
