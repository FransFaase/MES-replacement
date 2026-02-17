#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#ifdef __TCC_CC__

#define NULL 0
#define STDOUT_FILENO 1

char **_sys_env = 0;
ssize_t sys_syscall(size_t a, size_t b, size_t c, size_t d);
#include "sys_syscall.h"
void *sys_malloc(size_t size);

#define O_RDONLY 0
#define O_WRITE 001101

#define open(pathname, mode) sys_syscall(__NR_open, pathname, mode, 0777)
#define close(fd) sys_syscall(__NR_close, fd, 0, 0)
#define read(fd, buf, count) sys_syscall(__NR_read, fd, buf, count)
#define write(fd, buf, count) sys_syscall(__NR_write, fd, buf, count)
#define chmod(fn, mode) sys_syscall(__NR_chmod, fn, mode, 0)

void *malloc(size_t size) { return sys_malloc((size + 3) & ~3); }

int strcmp(const char *s1, const char *s2)
{
	for (;;)
	{
		int result = *s1 - *s2;
		if (result != 0 || *s1 == 0)
			return result;
		s1++;
		s2++;
	}
	return 0; // should not get here
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	for (; n > 0; n--)
	{
		int result = *s1 - *s2;
		if (result != 0 || *s1 == 0)
			return result;
		s1++;
		s2++;
	}
	return 0;
}

size_t strlen(const char *s)
{
	int len = 0;
	for (; *s != '\0'; s++)
		len++;
	return len;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	char *d = (char *)dest;
	char *s = (char *)src;
	for (int i = 0; i < n; i++)
	{
		d[i] = s[i];
		if (s[i] == '\0')
			break;
	}
}

#else

#define O_WRITE (O_WRONLY | O_CREAT | O_TRUNC)

#endif

void fhputc(int ch, int fh)
{
    write(fh, &ch, 1);
}

int fhgets(char *buffer, int size, int fh)
{
    int i = 0;
    while (i < size - 1)
    {
        char ch;
        if (read(fh, &ch, 1) == 0)
        {
            if (i == 0)
                return 0;
            break;
        }
        buffer[i++] = ch;
        if (ch == '\n')
            break;
    }
    buffer[i] = '\0';
    return 1;
}

int ip = 0;

int is_hex(char ch)
{
    if ('0' <= ch && ch <= '9')
        return ch - '0';
    if ('A' <= ch && ch <= 'F')
        return ch - ('A' - 10);
    return -1;
}

typedef struct label_s *label_p;
struct label_s
{
    char *name;
    int ip;
    label_p next;
};
label_p labels = NULL;

int pos_for_label(const char *name, int len)
{
    for (label_p label = labels; label != NULL; label = label->next)
        if (strncmp(label->name, name, len) == 0 && label->name[len] == '\0')
            return label->ip;
    return 0;
}

void process_file(const char *name, int add_labels, void (*output_byte)(unsigned char, int), void (*end_of_line)(const char *s))
{
    int f = open(name, O_RDONLY);
    if (f < 0)
        return;
    static char line[1000];
    int line_nr = 0;
    while (fhgets(line, 999, f))
    {
        line_nr++;
        int space = 0;
        char *s = line;
        while (*s != '\0' && *s != '#' && *s != '\r' && *s != '\n')
            if (*s <= ' ')
            {
                space = 1;
                s++;
            }
            else if (*s == ':')
            {
                s++;
                char *label = s;
                int label_len = 0;
                for (; *s > ' '; s++)
                    label_len++;
                if (label_len > 0 && add_labels)
                {
                    label_p new_label = (label_p)malloc(sizeof(struct label_s));
                    new_label->name = (char*)malloc(label_len + 1);
                    strncpy(new_label->name, label, label_len);
                    new_label->ip = ip;
                    new_label->next = labels;
                    labels = new_label;
                }
            }
            else if (is_hex(*s) >= 0 && is_hex(s[1]) >= 0)
            {
                unsigned byte = (is_hex(*s) << 4) | is_hex(s[1]);
                if (output_byte != 0)
                    output_byte(byte, space);
                space = 0;
                ip++;
                s += 2;
            }
            else if (*s == '&' || *s == '%' || *s == '!')
            {
                char mode = *s++;
                int l = 0;
                while (s[l] > ' ' && s[l] != '#' && s[l] != '>')
                    l++;
                int pos = pos_for_label(s, l);
                s += l;
                ip += 4;
                if (mode == '%')
                {
                    if (*s == '>')
                    {
                        s++;
                        l = 0;
                        while (s[l] > ' ' && s[l] != '#')
                            l++;
                        pos -= pos_for_label(s, l);
                        s += l;
                    }
                    else
                        pos -= ip;
                }
                int nr_bits = mode == '!' ? 8 : 32;
                for (int i = 0; i < nr_bits; i += 8)
                {
                    unsigned byte = (pos >> i) & 0xff;
                    if (output_byte != 0)
                        output_byte(byte, space);
                    space = 0;
                }
            }
            else if (*s == '"')
            {
                s++;
                while (*s != '\0' && *s != '"')
                {
                    if (output_byte != 0)
                        output_byte(*s, space);
                    space = 0;
                    s++;
                    ip++;
                }
                if (*s == '"')
                    s++;
                if (output_byte != 0)
                    output_byte('\0', space);
                space = 0;
                ip++;
            }
            else
            {
#ifndef __TCC_CC__
                fprintf(stderr, "%s:%d: Unknown '%c'(%d) '%s'\n", name, line_nr, *s, *s, line);
#endif
                return;
            }
        while (*s != '\0' && *s != '#' && *s != '\r' && *s != '\n')
            s++;
        if (end_of_line != 0)
            end_of_line(s);
    }
    close(f);
}

void output_hex(char ch, int fh)
{
    fhputc(ch + (ch < 10 ? '0' : 'A' - 10), fh);
}

int col = 1;
int fout = -1;

void output_hex_byte(unsigned char byte, int space)
{
    if (space)
    {
        fhputc(' ', fout);
        col++;
    }
    output_hex((byte >> 4) & 0xf, fout);
    output_hex(byte & 0xf, fout);
    col += 2;
}

void output_hex_end_of_line(const char *s)
{
    for (; col < 30; col++)
        fhputc(' ', fout);
    while (*s != '\0' && *s != '\r' && *s != '\n')
        fhputc(*s++, fout);
    fhputc('\n', fout);
    col = 1;
}

void output_byte(unsigned char byte, int space)
{
    (void)space;
    fhputc(byte, fout);
}

int main(int argc, char *argv[])
{
    fout = STDOUT_FILENO;
    int output_as_hex = 1;
    char *input_files[10];
    int nr_input_files = 0;
    char *output_file = NULL;
    for (int i  = 1; i < argc; i++)
        if (strcmp(argv[i], "-o") == 0)
        {
            i++;
            output_file = argv[i]; 
        }
        else
            input_files[nr_input_files++] = argv[i];

    if (output_file != NULL)
    {
        fout = open(output_file, O_WRITE);
        if (fout < 0)
        {
#ifndef __TCC_CC__            
            fprintf(stderr, "Error: Cannot open file '%s' for writing", output_file);
#endif
            return -1;
        }
        int len = strlen(output_file);
        output_as_hex = len > 4 && strcmp(output_file + len - 4, "hex0") == 0;
    }

    ip = 0x8048000;
    for (int i = 0; i < nr_input_files; i++)
        process_file(input_files[i], 1, 0, 0);

    ip = 0x8048000;
    for (int i = 0; i < nr_input_files; i++)
        if (output_as_hex)
            process_file(input_files[i], 0, output_hex_byte, output_hex_end_of_line);
        else
            process_file(input_files[i], 0, output_byte, 0);

    if (output_file != NULL)
    {
        close(fout);
        chmod(output_file, 0777);
    }

    return 0;
}