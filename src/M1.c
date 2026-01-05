#include <stdio.h>
#include <malloc.h>
#include <string.h>

#ifdef __TCC_CC__

int fgets(char *buffer, int size, FILE *f)
{
    int i = 0;
    while (i < size - 1)
    {
        char ch;
        if (read(f->fh, &ch, 1) == 0)
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

#endif


typedef struct define_s *define_p;
struct define_s
{
    char name[20];
    char value[20];
    define_p next;
};
define_p defines = 0;

void add_define(const char* name, const char *value)
{
    define_p new_define = (define_p)malloc(sizeof(struct define_s));
    strncpy(new_define->name, name, 19);
    new_define->name[19] = '\0';
    strncpy(new_define->value, value, 19);
    new_define->value[19] = '\0';
    new_define->next = defines;
    //printf("define '%s' %s'\n", new_define->name, new_define->value);
    defines = new_define;
}

const char* find_define(const char* name)
{
    for (define_p def = defines; def != NULL; def = def->next)
        if (strcmp(def->name, name) == 0)
            return def->value;
    return name;
}

void output_hex(char ch, FILE *f)
{
    fputc(ch + (ch < 10 ? '0' : 'A' - 10), f);
}

int main(int argc, char *argv[])
{
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

    FILE *fout = stdout;
    if (output_file != NULL)
    {
        fout = fopen(output_file, "w");
        if (fout == NULL)
        {
            fprintf(stderr, "Error: Cannot open file '%s' for writing", output_file);
            return -1;
        }
    }

    for (int i = 0; i < nr_input_files; i++)
    {
        FILE *fin = fopen(input_files[i], "r");
        if (fin == NULL)
        {
            fprintf(stderr, "Cannot open %s for reading\n", input_files[i]);
            return -1;
        }

        static char line[100000];
        while (fgets(line, 99999, fin))
        {
            if (strncmp(line, "DEFINE ", 7) == 0)
            {
                char *s = line + 7;
                while (*s == ' ') s++;
                char *name = s;
                while (*s > ' ')
                    s++;
                if (*s <= ' ')
                    *s++ = '\0';
                while (*s == ' ') s++;
                char *value = s;
                while (*s > ' ')
                    s++;
                if (*s <= ' ')
                    *s = '\0';
                add_define(name, value);
            }
            else
            {
                char *s = line;
                while (*s == ' ' || *s == '\t')
                    s++;
                while (*s != '\0' && *s != '\r' && *s != '\n' && *s != '#')
                {
                    if (*s <= ' ')
                    {
                        fputc(' ', fout);
                        s++;
                    }
                    else if ((*s == '%' || *s == '!') && (('0' <= s[1] && s[1] <= '9') || s[1] == '-'))
                    {
                        int nr_bits = *s == '%' ? 32 : 8;
                        s++;
                        int sign = 1;
                        if (*s == '-')
                        {
                            sign = -1;
                            s++;
                        }
                        int v = 0;
                        for (; '0' <= *s && *s <= '9'; s++)
                            v = 10 * v + *s - '0';
                        v *= sign;
                        for (int i = 0; i < nr_bits; i += 8)
                        {
                            output_hex((v >> (i + 4)) & 0xf, fout);
                            output_hex((v >> i)  & 0xf, fout);
                        }
                    }
                    else if (*s == '%' || *s == ':' || *s == '#' || *s == '&')
                    {
                        do
                            fputc(*s++, fout);
                        while (*s > ' ');
                    }
                    else if (*s == '"')
                    {
                        s++;
                        while (*s != '"' && *s != '\0')
                        {
                            int v = *s++;
                            output_hex((v >> 4) & 0xf, fout);
                            output_hex(v  & 0xf, fout);
                        }
                        fprintf(fout, "00 ");
                        if (*s == '"')
                            s++;
                    }
                    else
                    {
                        int i = 1;
                        while (s[i] > ' ')
                            i++;
                        char ch = s[i];
                        s[i] = '\0';
                        fprintf(fout, "%s", find_define(s));
                        s += i;
                        *s = ch;
                    }
                }
                fputs(" #", fout);
                for (char *s = line; *s != '\n'; s++)
                    if (s > line + 100)
                    {
                        fputs(" ...", fout);
                        break;
                    }
                    else
                        fputc(*s, fout);
                fputc('\n', fout);
            }
        }
        fclose(fout);
    }
}