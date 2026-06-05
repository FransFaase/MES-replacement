int sys_syscall(int a, int b, int c, int d);

#if defined(TCC_TARGET_ARM64) || defined(TCC_TARGET_RISCV64)
// aarch64 / riscv64 (asm-generic) syscall numbers. File-related calls that x86 exposes as
// open/access/mkdir/... only exist as the *at variants here; the numbers below
// are the *at syscalls, so the open()/access()/... wrappers need adapting in a
// later milestone. write/read/close/exit (used by tcc_s -version) are correct.

int sys_syscall4(int a, int b, int c, int d, int e);

#define AT_FDCWD -100
#define SIGCHLD    17
#define __NR_exit     93
#define __NR_fork    220  // clone
#define __NR_read     63
#define __NR_write    64
#define __NR_open     56  // openat
#define __NR_close    57
#define __NR_waitpid 260  // wait4
#define __NR_unlink   35  // unlinkat
#define __NR_execve  221
#define __NR_chdir    49
#define __NR_chmod    53  // fchmodat
#define __NR_lseek    62
#define __NR_access   48  // faccessat
#define __NR_mkdir    34  // mkdirat
#define __NR_symlink  36  // symlinkat
#define __NR_uname   160
#define __NR_getcwd   17

#define SYSCALL_FORK() sys_syscall(__NR_fork, SIGCHLD, 0, 0)
#define SYSCALL_OPEN(P,M,A) sys_syscall4(__NR_open, AT_FDCWD, P, M, A)
#define SYSCALL_WAITPID(F,S,M) sys_syscall4(__NR_waitpid, F, S, M, 0)
#define SYSCALL_UNLINK(P) sys_syscall(__NR_unlink, AT_FDCWD, P, 0)
#define SYSCALL_CHMOD(F,M) sys_syscall4(__NR_chmod, AT_FDCWD, F, M, 0)
#define SYSCALL_ACCESS(F,M)	sys_syscall(__NR_access, AT_FDCWD, F, M)
#define SYSCALL_MKDIR(P,M) sys_syscall(__NR_mkdir, AT_FDCWD, P, M)
#define SYSCALL_SYMLINK(T,L) sys_syscall(__NR_symlink, T, AT_FDCWD, L)

#else
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

#define SYSCALL_FORK() sys_syscall(__NR_fork, 0, 0, 0)
#define SYSCALL_OPEN(P,M,A) sys_syscall(__NR_open, P, M, A)
#define SYSCALL_WAITPID(F,S,M) sys_syscall(__NR_waitpid, F, S, M)
#define SYSCALL_UNLINK(P) sys_syscall(__NR_unlink, P, 0, 0)
#define SYSCALL_CHMOD(F,M) sys_syscall(__NR_chmod, F, M, 0)
#define SYSCALL_ACCESS(F,M)	sys_syscall(__NR_access, F, M, 0)
#define SYSCALL_MKDIR(P,M) sys_syscall(__NR_mkdir, P, M, 0)
#define SYSCALL_SYMLINK(T,L) sys_syscall(__NR_symlink, T, L, 0)

#endif

#define SYSCALL_EXIT(R) sys_syscall(__NR_exit, R, 0, 0)
#define SYSCALL_READ(F,B,C) sys_syscall(__NR_read, F, B, C)
#define SYSCALL_WRITE(F,B,C) sys_syscall(__NR_write, F, B, C)
#define SYSCALL_CLOSE(F) sys_syscall(__NR_close, F, 0, 0)
#define SYSCALL_EXECVE(P,A,E) sys_syscall(__NR_execve, P, A, E)
#define SYSCALL_CHDIR(P) sys_syscall(__NR_chdir, P, 0, 0)
#define SYSCALL_LSEEK(F,O,W) sys_syscall(__NR_lseek, F, O, W)
#define SYSCALL_UNAME(B) sys_syscall(__NR_uname, B, 0, 0)
#define SYSCALL_GETCWD(B,S) sys_syscall(__NR_getcwd, B, S, 0);

