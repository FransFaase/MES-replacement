#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __TCC_CC__

char **_sys_env = 0;
void *sys_malloc(size_t size);

#include "sys_syscall.h"

#define O_RDONLY 0

#define read(fd, buf, count) SYSCALL_READ(fd, buf, count)
#define open(pathname, mode) SYSCALL_OPEN(pathname, mode, 0777)
#define close(fd) SYSCALL_CLOSE(fd)
#define lseek(fd, offset, whence) SYSCALL_LSEEK(fd, offset, whence)

#endif

int main(int argc, char *argv[])
{
    int fh1 = open(argv[1], O_RDONLY);
    int fh2 = open(argv[2], O_RDONLY);

    size_t size = lseek(fh1, 0, 2);
    if (size != lseek(fh2, 0, 2))
        return -1;
    
    lseek(fh1, 0, 0);
    lseek(fh2, 0, 0);

    char *content1 = sys_malloc(size);
    char *content2 = sys_malloc(size);

    read(fh1, content1, size);
    read(fh2, content2, size);

    close(fh1);
    close(fh2);
    
    for (int i = 0; i < size; i++)
        if (content1[i] != content2[i])
            return -1;
    
    return 0;
}
