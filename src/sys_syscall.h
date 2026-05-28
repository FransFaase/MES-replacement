#ifdef __TCC_CC_32__
#define __NR_exit      1
#define __NR_fork      2
#define __NR_read      3
#define __NR_write     4
#define __NR_open      5
#define __NR_close     6
#define __NR_waitpid   7
#define __NR_unlink   10
#define __NR_execve   11
#define __NR_chdir    12
#define __NR_chmod    15
#define __NR_lseek    19
#define __NR_access   33
#define __NR_mkdir    39
#define __NR_symlink  83
#define __NR_uname   109
#define __NR_getcwd  183
#elif defined(TCC_TARGET_ARM64)
// aarch64 (asm-generic) syscall numbers. File-related calls that x86 exposes as
// open/access/mkdir/... only exist as the *at variants here; the numbers below
// are the *at syscalls, so the open()/access()/... wrappers need adapting in a
// later milestone. write/read/close/exit (used by tcc_s -version) are correct.
#define __NR_exit 93
#define __NR_fork 220   // clone
#define __NR_read 63
#define __NR_write 64
#define __NR_open 56    // openat
#define __NR_close 57
#define __NR_waitpid 260 // wait4
#define __NR_unlink 35  // unlinkat
#define __NR_execve 221
#define __NR_chdir 49
#define __NR_chmod 53   // fchmodat
#define __NR_lseek 62
#define __NR_access 48  // faccessat
#define __NR_mkdir 34   // mkdirat
#define __NR_symlink 36 // symlinkat
#define __NR_uname 160
#define __NR_getcwd 17
#else
#define __NR_exit 60
#define __NR_fork 57
#define __NR_read 0
#define __NR_write 1
#define __NR_open 2
#define __NR_close 3
#define __NR_waitpid 61 // wait4
#define __NR_unlink 87
#define __NR_execve 59
#define __NR_chdir 80
#define __NR_chmod 90
#define __NR_lseek 8
#define __NR_access 21
#define __NR_mkdir 83
#define __NR_symlink 88
#define __NR_uname 63
#define __NR_getcwd 79
#endif
