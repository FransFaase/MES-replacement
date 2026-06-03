
#include <fcntl.h>
#include <unistd.h>

#ifdef __TCC_CC__

char **_sys_env = 0;
#include "sys_syscall.h"

#define O_RDONLY 0

#define read(fd, buf, count) SYSCALL_READ(fd, buf, count)
#define open(pathname, mode) SYSCALL_OPEN(pathname, mode, 0777)
#define close(fd) SYSCALL_CLOSE(fd)

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