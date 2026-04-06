// Functions implemented in stack_c_intro.M1

ssize_t sys_syscall(ssize_t a, ssize_t b, ssize_t c, ssize_t d); // https://faculty.nps.edu/cseagle/assembly/sys_call.html
#include "sys_syscall.h"
void *sys_malloc(size_t size);

void exit(ssize_t result)
{
	sys_syscall(__NR_exit, result, 0, 0);
}

#define NULL 0

typedef ssize_t off_t;
typedef uint32_t time_t;
typedef uint32_t suseconds_t;

typedef struct 
{
	size_t fh;
	size_t at_eof;
} FILE;
FILE __sys_stdin = { 0, 0 };
FILE __sys_stdout = { 1, 0 };
FILE __sys_stderr = { 2, 0 };

const FILE *stdin = &__sys_stdin;
const FILE *stdout = &__sys_stdout;
const FILE *stderr = &__sys_stderr;

void *memcpy(void *dest, const void *src, size_t n)
{
	char *d = (char *)dest;
	char *s = (char *)src;
	for (size_t i = 0; i < n; i++)
		d[i] = s[i];
	return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
	char *d = (char *)dest;
	char *s = (char *)src;
	if (1) //dest < src)
	{
		for (size_t i = 0; i < n; i++)
			d[i] = s[i];
	}
	else if (src < dest)
	{
		size_t j = n-1;
		for (size_t i = 0; i < n; i++, j--)
			d[j] = s[j];
	}
	return dest;
}

void *memset(void *s, ssize_t c, size_t n)
{
	char *p = s
	for (size_t i = 0; i < n; i++)
		p[i] = c;
	return s;
}

ssize_t memcmp(const void *s1, const void *s2, size_t n)
{
	char *p1 = (char*)s1;
	char *p2 = (char*)s2;
	for (size_t i = 0; i < n; i++, p1++, p2++)
	{
		ssize_t result = *(char*)p1 - *(char*)p2;
		if (result != 0)
			return result;
	}
	return 0;
}

size_t strlen(const char *s)
{
	size_t len = 0;
	for (; *s != '\0'; s++)
		len++;
	return len;
}

char *strcpy(char *dest, const char *src)
{
	char *result = dest;
	while (*src != '\0')
		*dest++ = *src++;
	*dest = '\0';
	return result;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	char *d = (char *)dest;
	char *s = (char *)src;
	for (size_t i = 0; i < n; i++)
	{
		d[i] = s[i];
		if (s[i] == '\0')
			break;
	}
	return dest;
}

char *strcat(char *dest, const char *src)
{
	strcpy(dest + strlen(dest), src);
	return dest;
}

char *strchr(const char *s, ssize_t c)
{
	for (; *s != '\0'; s++)
		if (*s == c)
			return s;
	return c == '\0' ? s : NULL;
}

char *strrchr(const char *s, ssize_t c)
{
	const char *result = NULL;
	for (; *s != '\0'; s++)
		if (*s == c)
			result = s;
	return result;
}

ssize_t strcmp(const char *s1, const char *s2)
{
	for (;;)
	{
		ssize_t result = *s1 - *s2;
		if (result != 0 || *s1 == 0)
			return result;
		s1++;
		s2++;
	}
	return 0; // should not get here
}

ssize_t strncmp(const char *s1, const char *s2, size_t n)
{
	for (; n > 0; n--)
	{
		ssize_t result = *s1 - *s2;
		if (result != 0 || *s1 == 0)
			return result;
		s1++;
		s2++;
	}
	return 0;
}

char *strstr(const char *haystack, const char *needle)
{
	size_t n = strlen(needle);
	for (; *haystack != '\0'; haystack++)
		if (strncmp(haystack, needle, n) == 0)
			return haystack;
	return NULL;
}


ssize_t strtoul(const char *nptr, char **endptr, size_t base)
{
	if (base == 0)
	{
		base = 10;
		if (*nptr == '0')
		{
			base = 8;
			nptr++;
			if (*nptr == 'x' || *nptr == 'X')
			{
				base = 16;
				nptr++;
			}
		}
	}
	ssize_t result = 0;
	char sub_10 = '0' + (base < 10 ? base : 10);
	for (;; nptr++)
	{
		if ('0' <= *nptr && *nptr < sub_10)
			result = base * result + *nptr - '0';
		else if ('a' <= *nptr && *nptr < 'a' + base - 10)
			result = base * result + *nptr - 'a' + 10;
		else if ('A' <= *nptr && *nptr < 'A' + base - 10)
			result = base * result + *nptr - 'A' + 10;
		else
			break;
	}
	if (endptr != NULL)
		*endptr = nptr;
	return result;
}

ssize_t strtol(const char *nptr, char **endptr, size_t base)
{
	ssize_t sign = 1;
	if (*nptr == '-')
	{
		sign = -1;
		nptr++;
	}
	return sign * strtoul(nptr, endptr, base);
}

ssize_t strtoll(const char *nptr, char **endptr, size_t base)
{
	return strtol(nptr, endptr, base);
}

ssize_t strtoull(const char *nptr, char **endptr, size_t base)
{
	return strtoul(nptr, endptr, base);
}

float strtof(const char* str, char **endptr)
{
	// TODO: implement float parsing to int?
	*endptr = str;
	return 0;
}

void *malloc(size_t size)
{
#ifdef __TCC_CC_32__
	size = (size + 3) & ~3;
	size_t *result = sys_malloc(size + 4);
#else
	size = (size + 7) & ~7;
	size_t *result = sys_malloc(size + 8);
#endif
	*result = size;
	result++;
	return result;
}

void *realloc(void *ptr, size_t size)
{
	size_t *result = malloc(size);
	if (ptr != 0)
	{
		size_t *old_ptr = ptr;
#ifdef __TCC_CC_32__
		size_t old_size = old_ptr[-1] / 4;
#else
		size_t old_size = old_ptr[-1] / 8;
#endif
		for (size_t i = 0; i < old_size; i++)
			result[i] = old_ptr[i];
	}
	return result;
}

void *calloc(size_t N, size_t S)
{
	size_t len = N * S;
	char *r = (char*)malloc(len);
	for (size_t i = 0; i < len; i++)
		r[i] = '\0';
	return r;
}

void free(void *ptr)
{
	// Do freeing of memory
	return 0
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return sys_syscall(__NR_write, stream->fh, ptr, size * nmemb);
}

ssize_t fputc(ssize_t c, FILE *stream)
{
	return sys_syscall(__NR_write, stream->fh, &c, 1);
}

ssize_t fputs(const char *s, FILE *stream)
{
	return sys_syscall(__NR_write, stream->fh, s, strlen(s));
}

typedef ssize_t *va_list;

size_t __sys_printf(FILE *stream, char *trg, size_t len, char *format, va_list args)
{
	char buffer[40];
	size_t l = 0;
	char *s;
	size_t cnt = 0;

	char *org_format = format;

	for (;;)
	{
		if (len == 0)
			break;
		if (l > 0)
		{
			if (stream != 0)
				fputc(*s, stream);
			else if (trg != 0)
				*trg++ = *s;
			s++;
			l--;
			cnt++;
			if (len > 0)
				len--;
		}
		if (l == 0)
		{
			if (*format == '\0')
				break;
			s = buffer;
			l = 1;
			if (*format == '%')
			{
				format++;
				if (*format == '%')
					buffer[0] = '%';
				else
				{
					size_t modifier = 0;
					ssize_t sign = 1;
					if (*format == '-')
					{
						sign = -1;
						format++;
					}
					while ('0' <= *format && *format <= '9')
					{
						modifier = 10 * modifier + *format - '0';
						format++;
					}
					while (*format == 'l')
						format++;
					if (*format == 's')
					{
						s = (char*)*args++;
						l = strlen(s);
					}
					else if (*format == 'd')
					{
						ssize_t v = *args;
						size_t b = 40;
						if (v == 0)
							buffer[--b] = '0';
						else
						{
							size_t negative = v < 0;
							if (negative) v = -v;
							for (; v > 0; v = v / 10)
								buffer[--b] = '0' + v % 10;
							if (negative)
								buffer[--b] = '-';
						}
						l = 40 - b;
						if (modifier > 0)
						{
							if (modifier > l)
							{
								if (sign == -1)
								{
									for (size_t i = 40 - modifier; i < 40; i++)
										buffer[i] = b < 40 ? buffer[b++] : ' ';
									b = 40 - modifier;
								}
								else
									for (; l < modifier; l++)
										buffer[--b] = ' ';
							}
							l = modifier;
						}
						s = buffer + b;
						args++;
					}
					else if (*format == 'u')
					{
						size_t v = *args++;
						if (v == 0)
							buffer[0] = '0';
						else
						{
							l = 40;
							for (; v > 0; v = v / 10)
								buffer[--l] = '0' + v % 10;
							s = buffer + l;
							l = 40 - l;
						}
					}
					else if (*format == 'x' || *format == 'p')
					{
						size_t v = *args++;
						if (v == 0)
							buffer[0] = '0';
						else
						{
							l = 40;
							for (; v != 0; v >>= 4)
								buffer[--l] = ((v & 0xf) < 10 ? '0' : ('a' - 10)) + (v & 0xf);
							s = buffer + l;
							l = 40 - l;
						}
					}
					else if (*format == 'c')
					{
						buffer[0] = *args++;
					}
					else
					{
						fputs("__sys_printf %", stderr);
						fputc(*format, stderr);
						fputs(" ", stderr);
						fputs(org_format, stderr);
						fputs("\n", stderr);
						exit(1);
					}
				}
			}
			else
			{
				buffer[0] = *format;
				s = buffer;
				l = 1;
			}
			format++;
		}
	}
	if (len != 0 && trg != 0)
		*trg = '\0';
	return cnt;
}

void va_start(va_list ap, ...);
void va_end(va_list ap) {}

size_t fprintf(FILE *stream, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	return __sys_printf(stream, 0, -1, format, ap);
}

size_t printf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	return __sys_printf(stdout, 0, -1, format, ap);
}

size_t sprintf(char *str, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	return __sys_printf(0, str, -1, format, ap);
}

size_t snprintf(char *str, size_t size, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	return __sys_printf(0, str, size, format, ap);
}

size_t vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	return __sys_printf(0, str, size, format, ap);
}

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2
#define O_CREAT 00100
#define O_TRUNC 001000

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

ssize_t open(const char *filename, ssize_t flag, ...)
{
	size_t mode = 0;
	if ((flag & O_WRONLY) != 0)
	{
		va_list ap;
		va_start(ap, flag);
		mode = ap[0];
	}
	return sys_syscall(__NR_open, filename, flag, mode);
}

ssize_t close(ssize_t fd)
{
	return sys_syscall(__NR_close, fd, 0, 0);
}

size_t read(ssize_t fd, void *buf, size_t count)
{
	return sys_syscall(__NR_read, fd, buf, count);
}

off_t lseek(ssize_t fd, off_t offset, ssize_t whence)
{
	return sys_syscall(__NR_lseek, fd, offset, whence);
}

FILE *fopen(const char *pathname, const char *mode)
{
	char rw = *mode;
	if (*mode == 'r' || *mode == 'w')
		mode++;
	ssize_t bin = 0;
	if (*mode == 'b')
	{
		bin = 1;
		mode++;
	}
	ssize_t plus = 0;
	if (*mode == '+')
	{
		plus = 1;
		mode++;
	}
	if (*mode != '\0')
	{
		printf("Mode %s should be 'r/w(b)(+)', 'w', or 'wb'\n", mode);
		return 0;
	}

	ssize_t open_mode =   rw == 'r'
						? (plus == 1 ? O_RDWR : O_RDONLY)
						: ((plus == 1 ? O_RDWR : O_WRONLY) | O_CREAT | O_TRUNC);
	ssize_t fh = sys_syscall(__NR_open, pathname, open_mode, 0777);
	if (fh < 0)
	{
		printf("fopen %s %s returned %d\n", pathname, mode, fh);
		return 0;
	}
	FILE *f = (FILE*)malloc(sizeof(FILE));
	f->fh = fh;
	f->at_eof = 0;
	return f;
}

FILE *fdopen(ssize_t fd, const char *mode)
{
	FILE *f = (FILE*)malloc(sizeof(FILE));
	f->fh = fd;
	f->at_eof = 0;
	return f;
}

ssize_t fclose(FILE *stream)
{
	return sys_syscall(__NR_close, stream->fh, 0, 0);
}

ssize_t fflush(FILE *stream)
{
	// (No buffered output)
	return 0;
}

size_t fseek(FILE *stream, size_t offset, size_t whence)
{
	return lseek(stream->fh, offset, whence);
}

size_t ftell(FILE *stream)
{
	return lseek(stream->fh, 0, SEEK_CUR);
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	char *s = (char*)ptr;
	for (size_t i = 0; i < nmemb; i++)
	{
		size_t r = read(stream->fh, s, size);
		if (r < size)
			return i;
		s += size;
	}
	return nmemb;
}


#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
off_t lseek(size_t fd, off_t offset, size_t whence)
{
	return sys_syscall(__NR_lseek, fd, offset, whence);
}

ssize_t feof(FILE *stream)
{
	return stream->at_eof;
}

ssize_t fgetc(FILE *stream)
{
	if (stream->at_eof)
		return -1;
	unsigned char ch;
	ssize_t ret = sys_syscall(__NR_read, stream->fh, &ch, 1);
	if (ret <= 0)
	{
		stream->at_eof = 1;
		return -1;
	}
	return ch;
}


double ldexp(double x, size_t exp)
{
	double result = x;
	for (size_t i = 1; i < exp; i++)
		result *= x;
	return result;
}

time_t time(time_t *tloc)
{
	// TODO (function is used, but not called)
	fprintf(stderr, "TODO time\n"); exit(1);
}

struct tm {
	int tm_sec;    /* Seconds (0-60) */
	int tm_min;    /* Minutes (0-59) */
	int tm_hour;   /* Hours (0-23) */
	int tm_mday;   /* Day of the month (1-31) */
	int tm_mon;    /* Month (0-11) */
	int tm_year;   /* Year - 1900 */
	int tm_wday;   /* Day of the week (0-6, Sunday = 0) */
	int tm_yday;   /* Day in the year (0-365, 1 Jan = 0) */
	int tm_isdst;  /* Daylight saving time */
};
struct tm *localtime(const time_t *timep)
{
	// TODO (function is used, but not called)
	fprintf(stderr, "TODO localtime\n"); exit(1);
}

struct timeval {
	time_t      tv_sec;     /* seconds */
	suseconds_t tv_usec;    /* microseconds */
};
struct timezone {
	int tz_minuteswest;     /* minutes west of Greenwich */
	int tz_dsttime;         /* type of DST correction */
};
size_t gettimeofday(struct timeval *tv, struct timezone *tz)
{
	return 0;
}

ssize_t errno;

#define NULL 0

// This is a hack for an enum that is defined inside a struct
const int LINE_MACRO_OUTPUT_FORMAT_GCC = 0;
const int LINE_MACRO_OUTPUT_FORMAT_NONE = 1;
const int LINE_MACRO_OUTPUT_FORMAT_STD = 2;
const int LINE_MACRO_OUTPUT_FORMAT_P10 = 11;

// for tcc_cc.c
ssize_t write(size_t fd, char* buf, unsigned count)
{
	return sys_syscall(__NR_write, fd, buf, count);
}

size_t fileno(FILE *stream)
{
	return stream->fh;
}

#define __LINE__ 0
#define __file__ ""
#define __func__ ""

char *getcwd(char *buf, size_t size)
{
	sys_syscall(__NR_getcwd, buf, size, 0);
	return buf;
}

char **_sys_env = 0;

char *getenv(const char *name)
{
	size_t len = strlen(name);
	for (char **env = _sys_env; *env != NULL; env++)
		if (strncmp(*env, name, len) == 0 && (*env)[len] == '=')
			return (*env) + len + 1;
}

void qsort(void *base, size_t nmemb, size_t size, ssize_t (*compare)(const void *, const void *))
{
	// just implement a simple bubble sort
	for (size_t go = 1; go == 1;)
	{
		go = 0;
		for (size_t i = 0; i + 1 < nmemb; i++)
		{
			char *arg1 = (char*)base + i * size;
			char *arg2 = (char*)base + (1 + i) * size;
			ssize_t sign = compare(arg1, arg2);
			if (sign > 0)
			{
				go = 1;
				for (size_t j = 0; j < size; j++)
					if (j + sizeof(size_t) - 1 < size)
					{
						//printf("(swap int %d %d %d)", j, *(int*)(arg1 + j), *(int*)(arg2 + j));;
						size_t h = *(size_t*)(arg1 + j);
						*(size_t*)(arg1 + j) = *(size_t*)(arg2 + j);
						*(size_t*)(arg2 + j) = h;
						j += sizeof(size_t) - 1;
					}
					else
					{
						//printf("(swap char %d)", j);
						char h = ((char*)arg1)[j];
						((char*)arg1)[j] = ((char*)arg2)[j];
						((char*)arg2)[j] = h;
					}
			}
		}
	}
}

time_t time(time_t *tloc)
{
	// TODO (function is used, but not called)
	fprintf(stderr, "TODO time\n"); exit(1);
}

ssize_t setjmp(jmp_buf env)
{
	// TODO (function is used, but not called)
	fprintf(stderr, "TODO setjmp\n"); exit(1);
	return 0;
}

void longjmp(jmp_buf env, ssize_t val)
{
	// TODO (function is used, but not called)
	fprintf(stderr, "TODO longjmp\n"); exit(0);
	exit(-1);
}

ssize_t unlink(const char *pathname)
{
	return sys_syscall(__NR_unlink, pathname, 0, 0);
}

size_t sscanf(const char *str, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	size_t args_parsed = 0;

	while (*format != '\0')
		if (*str == '\0')
			break;
		else if (*format == '%')
		{
			format++;
			if (*format == 'd')
			{
				format++;
				size_t v = 0;
				while ('0' <= *str && *str <= '9')
				{
					v = 10 * v + *str - '0';
					str++;
				}
				**((ssize_t**)ap) = v;
				ap++;
				args_parsed++;
			}
			else
			{
				fprintf(stderr, "sscanf: format %%%c not supported\n", *format);
				exit(1);
			}
		}
		else
		{
			if (*format != *str)
				break;
			format++;
			str++;
		}
	return args_parsed;
}

ssize_t atoi(const char *nptr)
{
	// TODO (function is used, but not called)
	fprintf(stderr, "TODO atoi\n"); exit(1);
}

ssize_t remove(const char *pathname)
{
	return sys_syscall(__NR_unlink, pathname, 0, 0);
}

ssize_t execvp(const char *file, char * argv[])
{
	// TODO (function is used, but not called)
	fprintf(stderr, "TODO execvp\n"); exit(1);
}

ssize_t mkdir(const char *pathname, size_t mode)
{
	return sys_syscall(__NR_mkdir, pathname, mode, 0);
}

ssize_t chdir(const char *path)
{
	sys_syscall(__NR_chdir, path, 0, 0);
}

ssize_t access(const char *filename, ssize_t mode)
{
	return sys_syscall(__NR_access, filename, mode, 0);
}

ssize_t chmod(const char *filename, ssize_t mode)
{
	return sys_syscall(__NR_chmod, filename, mode, 0);
}

ssize_t symlink(const char *target, const char *linkpath)
{
	return sys_syscall(__NR_symlink, target, linkpath, 0);
}

struct utsname {
    char sysname[];    /* Operating system name (e.g., "Linux") */
    char nodename[];   /* Name within communications network
                          to which the node is attached, if any */
    char release[];    /* Operating system release
                          (e.g., "2.6.28") */
    char version[];    /* Operating system version */
    char machine[];    /* Hardware type identifier */
#ifdef _GNU_SOURCE
    char domainname[]; /* NIS or YP domain name */
#endif
};

ssize_t uname(struct utsname *buf)
{
	return sys_syscall(__NR_uname, buf, 0, 0);
}

ssize_t execve(char *program, char **argv, char **env)
{
	return sys_syscall(__NR_execve, program, argv, env);
}

char *fgets(char *str, size_t len, FILE *f)
{
	if (feof(f))
		return NULL;
	
	for (size_t i = 0; i < len - 1; i++)
	{
		ssize_t ch = fgetc(f);
		if (ch < 0)
		{
			str[i] = '\0';
			break;
		}
		str[i] = ch;
		if (ch == '\n')
		{
			str[i+1] = '\0';
			break;
		}
	}
	return str;
}


#define __linux__

#define EOF -1
#define EXIT_FAILURE -1
#define EXIT_SUCCESS 0

void assert(int v)
{ 
	// Just ignore these
}

void abort(void)
{
	// Just ignore these
}
