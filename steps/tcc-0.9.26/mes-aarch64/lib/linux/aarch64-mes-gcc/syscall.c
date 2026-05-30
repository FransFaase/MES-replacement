/* aarch64 (arm64) syscall primitives for the mes libc, compiled by tcc_s.
 *
 * Linux/aarch64 syscall ABI: number in x8, arguments in x0..x5, "svc #0",
 * result in x0.  tcc 0.9.26 has no arm64 assembler, so each wrapper is raw
 * instruction words via __asm__(".int 0x..").  At entry the C arguments are
 * already in x0.. per AAPCS64, so we move the syscall number (1st arg, x0) into
 * x8 and shift the remaining arguments down by one register.  No C "return" is
 * used: after "svc #0" the result sits in x0 and tcc's epilogue (mov sp,x29;
 * ldp x29,x30,[sp],#224; ret) never touches x0, so it flows out as the result.
 *
 * Encodings (verified with llvm-mc --triple=aarch64):
 *   mov x8,x0 = 0xaa0003e8   mov x3,x4 = 0xaa0403e3
 *   mov x0,x1 = 0xaa0103e0   mov x4,x5 = 0xaa0503e4
 *   mov x1,x2 = 0xaa0203e1   mov x5,x6 = 0xaa0603e5
 *   mov x2,x3 = 0xaa0303e2   svc #0    = 0xd4000001
 */

#include <errno.h>

// *INDENT-OFF*
long
__sys_call (long sys_call)
{
  __asm__ (".int 0xaa0003e8"); /* mov x8, x0 */
  __asm__ (".int 0xd4000001"); /* svc #0     */
}

long
__sys_call1 (long sys_call, long one)
{
  __asm__ (".int 0xaa0003e8"); /* mov x8, x0 */
  __asm__ (".int 0xaa0103e0"); /* mov x0, x1 */
  __asm__ (".int 0xd4000001"); /* svc #0     */
}

long
__sys_call2 (long sys_call, long one, long two)
{
  __asm__ (".int 0xaa0003e8"); /* mov x8, x0 */
  __asm__ (".int 0xaa0103e0"); /* mov x0, x1 */
  __asm__ (".int 0xaa0203e1"); /* mov x1, x2 */
  __asm__ (".int 0xd4000001"); /* svc #0     */
}

long
__sys_call3 (long sys_call, long one, long two, long three)
{
  __asm__ (".int 0xaa0003e8"); /* mov x8, x0 */
  __asm__ (".int 0xaa0103e0"); /* mov x0, x1 */
  __asm__ (".int 0xaa0203e1"); /* mov x1, x2 */
  __asm__ (".int 0xaa0303e2"); /* mov x2, x3 */
  __asm__ (".int 0xd4000001"); /* svc #0     */
}

long
__sys_call4 (long sys_call, long one, long two, long three, long four)
{
  __asm__ (".int 0xaa0003e8"); /* mov x8, x0 */
  __asm__ (".int 0xaa0103e0"); /* mov x0, x1 */
  __asm__ (".int 0xaa0203e1"); /* mov x1, x2 */
  __asm__ (".int 0xaa0303e2"); /* mov x2, x3 */
  __asm__ (".int 0xaa0403e3"); /* mov x3, x4 */
  __asm__ (".int 0xd4000001"); /* svc #0     */
}

long
__sys_call5 (long sys_call, long one, long two, long three, long four, long five)
{
  __asm__ (".int 0xaa0003e8"); /* mov x8, x0 */
  __asm__ (".int 0xaa0103e0"); /* mov x0, x1 */
  __asm__ (".int 0xaa0203e1"); /* mov x1, x2 */
  __asm__ (".int 0xaa0303e2"); /* mov x2, x3 */
  __asm__ (".int 0xaa0403e3"); /* mov x3, x4 */
  __asm__ (".int 0xaa0503e4"); /* mov x4, x5 */
  __asm__ (".int 0xd4000001"); /* svc #0     */
}

long
__sys_call6 (long sys_call, long one, long two, long three, long four, long five, long six)
{
  __asm__ (".int 0xaa0003e8"); /* mov x8, x0 */
  __asm__ (".int 0xaa0103e0"); /* mov x0, x1 */
  __asm__ (".int 0xaa0203e1"); /* mov x1, x2 */
  __asm__ (".int 0xaa0303e2"); /* mov x2, x3 */
  __asm__ (".int 0xaa0403e3"); /* mov x3, x4 */
  __asm__ (".int 0xaa0503e4"); /* mov x4, x5 */
  __asm__ (".int 0xaa0603e5"); /* mov x5, x6 */
  __asm__ (".int 0xd4000001"); /* svc #0     */
}
// *INDENT-ON*

long
_sys_call (long sys_call)
{
  long r = __sys_call (sys_call);
  if (r < 0)
    {
      errno = -r;
      r = -1;
    }
  else
    errno = 0;
  return r;
}

long
_sys_call1 (long sys_call, long one)
{
  long r = __sys_call1 (sys_call, one);
  if (r < 0)
    {
      errno = -r;
      r = -1;
    }
  else
    errno = 0;
  return r;
}

long
_sys_call2 (long sys_call, long one, long two)
{
  long r = __sys_call2 (sys_call, one, two);
  if (r < 0)
    {
      errno = -r;
      r = -1;
    }
  else
    errno = 0;
  return r;
}

long
_sys_call3 (long sys_call, long one, long two, long three)
{
  long r = __sys_call3 (sys_call, one, two, three);
  if (r < 0)
    {
      errno = -r;
      r = -1;
    }
  else
    errno = 0;
  return r;
}

long
_sys_call4 (long sys_call, long one, long two, long three, long four)
{
  long r = __sys_call4 (sys_call, one, two, three, four);
  if (r < 0)
    {
      errno = -r;
      r = -1;
    }
  else
    errno = 0;
  return r;
}

long
_sys_call5 (long sys_call, long one, long two, long three, long four, long five)
{
  long r = __sys_call5 (sys_call, one, two, three, four, five);
  if (r < 0)
    {
      errno = -r;
      r = -1;
    }
  else
    errno = 0;
  return r;
}
