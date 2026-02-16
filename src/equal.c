
#include <fcntl.h>
#include <unistd.h>

#ifdef __TCC_CC__

char **_sys_env = 0;
ssize_t sys_syscall(size_t a, size_t b, size_t c, size_t d);
#include "sys_syscall.h"

#define O_RDONLY 0

#define open(pathname, mode) sys_syscall(__NR_open, pathname, mode, 0777)
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