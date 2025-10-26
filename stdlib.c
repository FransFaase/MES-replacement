// Functions defined in stack_c_intro.M1

int sys_int80(int a, int b, int c, int d); // https://faculty.nps.edu/cseagle/assembly/sys_call.html
void *sys_malloc(size_t size);

void exit(int result)
{
	sys_int80(1, result, 0, 0);
}

#define NULL 0

typedef uint32_t off_t;
typedef uint32_t time_t;
typedef uint32_t suseconds_t;

typedef struct 
{
	uint32_t fh;
	uint32_t at_eof;
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
	for (int i = 0; i < n; i++)
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

void *memset(void *s, int c, size_t n)
{
	char *p = s
	for (size_t i = 0; i < n; i++)
		p[i] = c;
	return s;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	char *p1 = (char*)s1;
	char *p2 = (char*)s2;
	for (size_t i = 0; i < n; i++, p1++, p2++)
	{
		int result = *(char*)p1 - *(char*)p2;
		if (result != 0)
			return result;
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
	for (int i = 0; i < n; i++)
	{
		d[i] = s[i];
		if (s[i] == '\0')
			break;
	}
}

char *strchr(const char *s, int c)
{
	for (; *s != '\0'; s++)
		if (*s == c)
			return s;
	return c == '\0' ? s : NULL;
}

char *strrchr(const char *s, int c)
{
	int n = strlen(s);
	for (int i = n; i >= 0; i--)
		if (s[i] == c)
			return s + i;
	return NULL;
}

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

char *strstr(const char *haystack, const char *needle)
{
	int n = strlen(needle);
	for (; *haystack != '\0'; haystack++)
		if (strncmp(haystack, needle, n) == 0)
			return haystack;
	return NULL;
}


long strtoul(const char *nptr, char **endptr, int base)
{
	long result = 0;
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
	*endptr = nptr;
	return result;
}

long strtol(const char *nptr, char **endptr, int base)
{
	long sign = 1;
	if (*nptr == '-')
	{
		sign = -1;
		nptr++;
	}
	return sign * strtoul(nptr, endptr, base);
}

long strtoll(const char *nptr, char **endptr, int base)
{
	return strtol(nptr, endptr, base);
}

long strtoull(const char *nptr, char **endptr, int base)
{
	return strtoul(nptr, endptr, base);
}

float strtof(const char* str, char **endptr)
{
	// TODO
	//fprintf(stderr, "TODO strtof\n"); exit(1);
	*endptr = str;
	return 0;
}

void *malloc(size_t size)
{
	size = (size + 3) & ~3;
	int *result = sys_malloc(size + 4); //(sys_malloc(size + 3) + 3) & ~3;
	*result = size;
	result++;
	//memset(result, 0, size);
	//for (size_t i = 0; i < size; i++)
	//	((char*)result)[i] = 0;
	//return (result + 3) & ~3;
	return result;
}

void *realloc(void *ptr, size_t size)
{
	int *result = malloc(size);
	if (ptr != 0)
	{
		int *old_ptr = ptr;
		int old_size = old_ptr[-1] / 4;
		for (int i = 0; i < old_size; i++)
			result[i] = old_ptr[i];
	}
	return result;
}

void free(void *ptr)
{
	// Do freeing of memory
	return 0
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	return sys_int80(4, stream->fh, ptr, size * nmemb);
}

int fputc(int c, FILE *stream)
{
	int buffer[1];
	buffer[0] = c;
	return sys_int80(4, stream->fh, buffer, 1);
}
int fputs(const char *s, FILE *stream)
{
	return sys_int80(4, stream->fh, s, strlen(s));
}

typedef int *va_list;

int __sys_printf(FILE *stream, char *trg, int len, char *format, va_list args)
{
	char buffer[20];
	int l = 0;
	char *s;
	int cnt = 0;

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
				else if (*format == 's')
				{
					s = (char*)*args++;
					l = strlen(s);
				}
				else if (*format == 'd')
				{
					int v = *args;
					if (v == 0)
						buffer[0] = '0';
					else
					{
						if (v < 0) v = -v;
						l = 20;
						for (; v > 0; v = v / 10)
							buffer[--l] = '0' + v % 10;
						if (*args < 0)
							buffer[--l] = '-';
						s = buffer + l;
						l = 20 - l;
					}
					args++;
				}
				else if (*format == 'u')
				{
					unsigned int v = *args++;
					if (v == 0)
						buffer[0] = '0';
					else
					{
						l = 20;
						for (; v > 0; v = v / 10)
							buffer[--l] = '0' + v % 10;
						s = buffer + l;
						l = 20 - l;
					}
				}
				else if (*format == 'x' || *format == 'p')
				{
					unsigned int v = *args++;
					if (v == 0)
						buffer[0] = '0';
					else
					{
						l = 20;
						for (; v != 0; v >>= 4)
							buffer[--l] = ((v & 0xf) < 10 ? '0' : ('a' - 10)) + (v & 0xf);
						s = buffer + l;
						l = 20 - l;
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
					fputs("\n", stderr);
					exit(1);
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

int fprintf(FILE *stream, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	return __sys_printf(stream, 0, -1, format, ap);
}

int printf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	return __sys_printf(stdout, 0, -1, format, ap);
}

int sprintf(char *str, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	return __sys_printf(0, str, -1, format, ap);
}
int snprintf(char *str, size_t size, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	return __sys_printf(0, str, size, format, ap);
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	return __sys_printf(0, str, size, format, ap);
}

#define O_RDONLY 0
#define O_WRONLY 1
#define O_CREAT 00100
#define O_TRUNC 001000

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

int open(const char *filename, int flag, ...)
{
	int mode = 0;
	if ((flag & O_WRONLY) != 0)
	{
		va_list ap;
		va_start(ap, flag);
		mode = ap[0];
	}
	return sys_int80(5, filename, flag, mode);
}

int close(int fd)
{
	return sys_int80(6, fd, 0, 0);
}

size_t read(int fd, void *buf, size_t count)
{
	return sys_int80(3, fd, buf, count);
}

off_t lseek(int fd, off_t offset, int whence)
{
	return sys_int80(19, fd, offset, whence);
}

FILE *fopen(const char *pathname, const char *mode)
{
	int open_mode = 0;
	if (mode[0] == 'r' && mode[1] == '\0')
		open_mode = O_RDONLY;
	else if (mode[0] == 'w' && mode[1] == '\0')
		open_mode = O_WRONLY | O_CREAT | O_TRUNC;
	else
	{
		printf("Mode %s should be 'r' or 'w'\n", mode);
		return 0;
	}
	int fh = sys_int80(5, pathname, open_mode, 0777);
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

FILE *fdopen(int fd, const char *mode)
{
	// TODO
	fprintf(stderr, "TODO fdopen\n"); exit(1);
}

int fclose(FILE *stream)
{
	return sys_int80(6, stream->fh, 0, 0);
}

int fflush(FILE *stream)
{
	// (No buffered output)
	return 0;
}

int fseek(FILE *stream, long offset, int whence)
{
	// TODO
	fprintf(stderr, "TODO fseek\n"); exit(1);
}

long ftell(FILE *stream)
{
	// TODO
	fprintf(stderr, "TODO ftell\n"); exit(1);
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	// TODO
	fprintf(stderr, "TODO fread\n"); exit(1);
}


#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
off_t lseek(int fd, off_t offset, int whence)
{
	return sys_int80(19, fd, offset, whence);
}

int feof(FILE *stream)
{
	return stream->at_eof;
}

int fgetc(FILE *stream)
{
	if (stream->at_eof)
		return -1;
	char buffer[0];
	int ret = sys_int80(3, stream->fh, buffer, 1);
	if (ret <= 0)
	{
		stream->at_eof = 1;
		return -1;
	}
	return buffer[0];
}


double ldexp(double x, int exp)
{
	double result = x;
	for (int i = 1; i < exp; i++)
		result *= x;
	return result;
}

time_t time(time_t *tloc)
{
	// TODO
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
	// TODO
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
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	return 0;
}

int errno;

#define NULL 0

// This is a hack for an enum that is defined inside a struct
const int LINE_MACRO_OUTPUT_FORMAT_GCC = 0;
const int LINE_MACRO_OUTPUT_FORMAT_NONE = 1;
const int LINE_MACRO_OUTPUT_FORMAT_STD = 2;
const int LINE_MACRO_OUTPUT_FORMAT_P10 = 11;

// for tcc_cc.c
int write(int fd, char* buf, unsigned count)
{
	return sys_int80(4, fd, buf, count);
}

int fileno(FILE *stream)
{
	return stream->fh;
}

#define __LINE__ 0
#define __file__ ""
#define __func__ ""

char *getcwd(char *buf, size_t size)
{
	// TODO
	fprintf(stderr, "TODO getcwd\n"); exit(1);
}

char **_sys_env = 0;

char *getenv(const char *name)
{
	int len = strlen(name);
	for (char **env = _sys_env; *env != NULL; env++)
		if (strncmp(*env, name, len) == 0 && (*env)[len] == '=')
			return (*env) + len + 1;
}

void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
	// TODO
	fprintf(stderr, "TODO qsort\n"); exit(1);
}

time_t time(time_t *tloc)
{
	// TODO
	fprintf(stderr, "TODO time\n"); exit(1);
}

int setjmp(jmp_buf env)
{
	// TODO
	fprintf(stderr, "TODO setjmp\n"); exit(1);
	return 0;
}

void longjmp(jmp_buf env, int val)
{
	// TODO
	fprintf(stderr, "TODO longjmp\n"); exit(0);
	exit(-1);
}

int unlink(const char *pathname)
{
	// TODO
	fprintf(stderr, "TODO unlink\n"); exit(1);
}

int sscanf(const char *str, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	int args_parsed = 0;

	while (*format != '\0')
		if (*str == '\0')
			break;
		else if (*format == '%')
		{
			format++;
			if (*format == 'd')
			{
				format++;
				int v = 0;
				while ('0' <= *str && *str <= '9')
				{
					v = 10 * v + *str - '0';
					str++;
				}
				**((int**)ap) = v;
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

int atoi(const char *nptr)
{
	// TODO
	fprintf(stderr, "TODO atoi\n"); exit(1);
}

int remove(const char *pathname)
{
	// TODO
	fprintf(stderr, "TODO remove\n"); exit(1);
}

int execvp(const char *file, char * argv[])
{
	// TODO
	fprintf(stderr, "TODO execvp\n"); exit(1);
}


void __init_globals__(void);

#define __linux__
