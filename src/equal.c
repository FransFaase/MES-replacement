
#include <fcntl.h>
#include <unistd.h>

#ifdef __TCC_CC__

char **_sys_env = 0;
int sys_int80(int a, int b, int c, int d);

#define O_RDONLY 0

#define open(pathname, mode) sys_int80(5, pathname, mode, 0777)
#define read(fd, buf, count) sys_int80(3, fd, buf, count)
#define close(fd) sys_int80(6, fd, 0, 0)

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