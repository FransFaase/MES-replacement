
#include <fcntl.h>
#include <unistd.h>

#ifdef __TCC_CC__

char **_sys_env = 0;
int sys_syscall(int a, int b, int c, int d);
int sys_syscall4(int a, int b, int c, int d, int e);
#include "sys_syscall.h"

#define O_RDONLY 0

#include "sys_syscall.h"
#ifdef TCC_TARGET_ARM64
// aarch64 only has openat (a 4-arg *at syscall with a leading dirfd); AT_FDCWD
// resolves the path relative to the cwd. sys_syscall4 carries the extra arg in x3.
#define AT_FDCWD -100
#define open(pathname, mode) sys_syscall4(__NR_open, AT_FDCWD, pathname, mode, 0777)
#else
#define open(pathname, mode) sys_syscall(__NR_open, pathname, mode, 0777)
#endif
#define read(fd, buf, count) sys_syscall(__NR_read, fd, buf, count)
#define close(fd) sys_syscall(__NR_close, fd, 0, 0)

#endif

int fhgetc(int fh)
{
    unsigned char ch;
    if (read(fh, &ch, 1) != 1)
        return -1;
    return ch;
}

int main(int argc, char *argv[])
{
    int fh1 = open(argv[1], O_RDONLY);
    int fh2 = open(argv[2], O_RDONLY);

    int result = -1;
    for (;;)
    {
        int ch = fhgetc(fh1);
        if (fhgetc(fh2) != ch)
            break;
        if (ch == -1)
        {
            result = 0;
            break;
        }
    }

    close(fh1);
    close(fh2);
    
    return result;
}