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
void *sys_malloc(size_t size);

#define O_RDONLY 0
#define O_WRITE 001101

#include "sys_syscall.h"
#define read(fd, buf, count) SYSCALL_READ(fd, buf, count)
#define write(fd, buf, count) SYSCALL_WRITE(fd, buf, count)
#define open(pathname, mode) SYSCALL_OPEN(pathname, mode, 0777)
#define close(fd) SYSCALL_CLOSE(fd)
#define chmod(fn, mode) SYSCALL_CHMOD(fn, mode)
#define lseek(fd, offset, whence) SYSCALL_LSEEK(fd, offset, whence)

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

#define LABEL_BUCKETS 1024

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

int my_left = 0;
char *my_mem = 0;
#define ALLOC_SIZE 100000

void* my_malloc(int size)
{
    if (my_left < size)
    {
        my_mem = (char*)malloc(ALLOC_SIZE);
        my_left = ALLOC_SIZE;
    }
    void *result = my_mem;
    my_mem += size;
    my_left -= size;
    return result;
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
};

unsigned int hash(const char *name, int len)
{
    unsigned int h = 0;
    for (int i = 0; i < len; i++) h = (h << 5) + h + name[i];
    return h;
}

typedef struct hexa_hash_tree_t *hexa_hash_tree_p;
struct hexa_hash_tree_t
{	char has_children;
	union
	{	label_p label;
		hexa_hash_tree_p *children;
	} data;
};

static hexa_hash_tree_p hash_tree = 0;

label_p find_label(const char* name, int len)
{
	hexa_hash_tree_p *r_node = &hash_tree;
	const char *vs = name;
	int depth;
	int mode = 0;

	for (depth = 0; ; depth++)
	{
        hexa_hash_tree_p node = *r_node;

		if (node == 0)
		{	node = (hexa_hash_tree_p)my_malloc(sizeof(struct hexa_hash_tree_t));
			node->has_children = 0;
            label_p label = (label_p)my_malloc(sizeof(struct label_s));
            label->ip = 0;
            label->name = (char*)my_malloc(len + 1);
            strncpy(label->name, name, len);
            label->name[len] = '\0';
			node->data.label = label;
			*r_node = node;
			return label;
		}

		if (node->has_children == 0)
		{	char *cs = node->data.label->name;
			hexa_hash_tree_p *children;
			unsigned short i, v = 0;

            for (i = 0; i < len; i++)
                if (cs[i] != name[i])
                    break;
			if (i == len && cs[len] == '\0')
				return node->data.label;

			children = (hexa_hash_tree_p*)my_malloc(16*sizeof(hexa_hash_tree_p*));
			for (i = 0; i < 16; i++)
				children[i] = 0;

			i = strlen(cs);
			if (depth <= i)
				v = ((unsigned char)cs[depth]) & 15;
			else if (depth <= i*2)
				v = ((unsigned char)cs[depth-i-1]) >> 4;

			children[v] = node;

			node = (hexa_hash_tree_p)my_malloc(sizeof(struct hexa_hash_tree_t));
			node->has_children = 1;
			node->data.children = children;
			*r_node = node;
		}
		{
            unsigned short v = 0;
            if (depth > 2 * len)
                ;
			else if (depth == len)
			{
                v = 0;
				mode = 1;
				vs = name;
			}
			else if (mode == 0)
				v = ((unsigned short)*vs++) & 15;
			else
				v = ((unsigned short)*vs++) >> 4;

			r_node = &node->data.children[v];
		}
	}
}

int pos_for_label(const char *name, int len)
{
    label_p v_label = find_label(name, len);
    return v_label->ip;
}

struct file_t
{
    char *name;
    int exists;
    int size;
    char *content;
};

void process_file(struct file_t *input_file, int add_labels, void (*output_byte)(unsigned char, int), void (*end_of_line)(const char *s))
{
    if (input_file->exists == -1)
    {
        input_file->exists = 0;
        int f = open(input_file->name, O_RDONLY);
        if (f >= 0)
        {
            input_file->exists = 1;
            input_file->size = lseek(f, 0, 2);
            input_file->content = malloc(input_file->size + 1);
            lseek(f, 0, 0);
            read(f, input_file->content, input_file->size);
            input_file->content[input_file->size] = '\0';
            close(f);
        }
    }
    if (input_file->exists == 0)
        return;

    int line_nr = 0;
    char *s = input_file->content;
    while (*s != '\0')
    {
        line_nr++;
        int space = 0;
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
                    label_p new_label = find_label(label, label_len);
                    new_label->ip = ip;
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
                fprintf(stderr, "%s:%d: Unknown '%c'(%d)\n", input_file->name, line_nr, *s, *s);
#endif
                return;
            }
        if (end_of_line != 0)
            end_of_line(s);
        while (*s != '\0' && *s != '\n')
            s++;
        if (*s == '\n')
            s++;
    }
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

char *buf_pos;

void output_byte(unsigned char byte, int space)
{
    (void)space;
    *buf_pos++ = byte;
}

int main(int argc, char *argv[])
{
    fout = STDOUT_FILENO;
    int output_as_hex = 1;
    struct file_t input_files[10];
    int nr_input_files = 0;
    char *output_file = NULL;
    for (int i  = 1; i < argc; i++)
        if (strcmp(argv[i], "-o") == 0)
        {
            i++;
            output_file = argv[i]; 
        }
        else
        {
            input_files[nr_input_files].exists = -1;
            input_files[nr_input_files++].name = argv[i];
        }

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
        output_as_hex = len > 5 && strcmp(output_file + len - 4, "hex0") == 0;
    }

    ip = 0x8048000;
    for (int i = 0; i < nr_input_files; i++)
        process_file(&input_files[i], 1, 0, 0);

    int bin_size = ip - 0x8048000;

    ip = 0x8048000;

    char *output_buf;
    if (!output_as_hex)
    {
        output_buf = (char *)malloc(bin_size);
        buf_pos = output_buf;
    }

    for (int i = 0; i < nr_input_files; i++)
        if (output_as_hex)
            process_file(&input_files[i], 0, output_hex_byte, output_hex_end_of_line);
        else
            process_file(&input_files[i], 0, output_byte, 0);

    if (!output_as_hex)
        write(fout, output_buf, bin_size);

    if (output_file != NULL)
    {
        close(fout);
        chmod(output_file, 0777);
    }

    return 0;
}
