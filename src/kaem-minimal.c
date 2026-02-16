#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#ifdef __TCC_CC__

char **_sys_env = 0;
ssize_t sys_syscall(size_t a, size_t b, size_t c, size_t d);
#include "sys_syscall.h"

#define O_RDONLY 0

#define open(pathname, mode) sys_syscall(__NR_open, pathname, mode, 0777)
#define read(fd, buf, count) sys_syscall(__NR_read, fd, buf, count)
#define fork() sys_syscall(__NR_fork, 0, 0, 0)
#define execve(program, argv, env) sys_syscall(__NR_execve, program, argv, env)
#define waitpid(f, status, mode) sys_syscall(__NR_waitpid, f, status, mode)

#endif

int fhgetc(int fh)
{
	char ch;
	int result = read(fh, &ch, 1);
	return result == 1 ? ch : -1;
}

int main(int argc, const char *argv[])
{
    const char *input_fn = "kaem.x86";
    if (argc > 1)
    {
        input_fn = argv[1];
    }

    int fh = open(input_fn, O_RDONLY);
    if (fh == 0)
    {
        return 1;
    }

    char **env = (char **)argv + (argc + 1);

    for (;;)
    {
        char line[200];
        char *s = line;
        int ch;
        for (;;)
        {
            ch = fhgetc(fh);
            if (ch == -1 || ch == '\n' || ch == '#')
                break;
            *s++ = ch;
        }
        *s = '\0';
        
        char *tokens[10];
        int nr_tokens = 0;
        s = line;

        for (;;)
        {
            while (*s == ' ' || *s == '\t')
                s++;
            if (*s == '\0')
                break;
            tokens[nr_tokens++] = s;
            while (*s > ' ')
                s++;
            if (*s == '\0')
                break;
            *s++ = '\0';
        }
        tokens[nr_tokens] = 0;

        if (nr_tokens > 0)
        {
            // Execute command
            int f = fork();
            if (f == -1)
            {
                return 1;
            }
            if (f == 0)
            {
                // We are child
                execve(tokens[0], tokens, env);
                return 1;
            }
            
            // We are parent
            int status;
            waitpid(f, &status, 0);
            if (status != 0)
            {
                return status;
            }
        }

        if (ch == '#')
        {
            do
            {
                ch = fhgetc(fh);
            } while (ch != -1 && ch != '\n');
        }
        if (ch == -1)
            break;
    }
    return 0;
}
