#ifdef __i386__
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
