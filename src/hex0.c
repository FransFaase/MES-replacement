#include <fcntl.h>
#include <unistd.h>

#ifdef __TCC_CC__

char **_sys_env = 0;
ssize_t sys_syscall(size_t a, size_t b, size_t c, size_t d);
#include "sys_syscall.h"

#define O_RDONLY 0
#define O_WRITE 001101

#define open(pathname, mode) sys_syscall(__NR_open, pathname, mode, 0777)
#define close(fd) sys_syscall(__NR_close, fd, 0, 0)
#define read(fd, buf, count) sys_syscall(__NR_read, fd, buf, count)
#define write(fd, buf, count) sys_syscall(__NR_write, fd, buf, count)
#define chmod(fn, mode) sys_syscall(__NR_chmod, fn, mode, 0)

#else

#define O_WRITE (O_WRONLY | O_CREAT | O_TRUNC)

#endif

int main(int argc, char *argv[])
{
    int fin = open(argv[1], O_RDONLY);
    int fout = open(argv[2], O_WRITE);

    int state = 0;
    int first = 0;
    unsigned char ch;
    while (read(fin, &ch, 1))
    {
        if (ch <= ' ')
            ;
        else if (ch == '#')
        {
            while (read(fin, &ch, 1) && ch != '\n')
                ;
        }
        else
        {
            if ('0' <= ch && ch <= '9')
                ch -= '0';
            else
                ch -= 'A' - 10;
            if (state)
            {
                ch += first;
                write(fout, &ch, 1);
                state = 0;
            }
            else
            {
                first = ch << 4;
                state = 1;
            }
        }
    }

    close(fout);
    chmod(argv[2], 0777);
}
