// Functions defined in stack_c_intro.M1

int sys_int80(int a, int b, int c, int d); // https://faculty.nps.edu/cseagle/assembly/sys_call.html
int sys_malloc(size_t size);

void exit(int result)
{
	sys_int80(1, result, 0, 0);
}

#define NULL 0

typedef uint32_t size;
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
	for (int i = 0; i < n; i++)
		dest[i] = src[i];
	return dest;
}
#if 0
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
#endif

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
#if 0
char *strncpy(char *dest, const char *src, size_t n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);
#endif
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
#if 0
int strncmp(const char *s1, const char *s2, size_t n);


long strtol(const char *nptr, char **endptr, int base);
long strtoul(const char *nptr, char **endptr, int base);
long strtoll(const char *nptr, char **endptr, int base);
long strtoull(const char *nptr, char **endptr, int base);
#endif

void *malloc(size_t size)
{
	uint32_t result = sys_malloc(size + 3);
	//return (result + 3) & ~3;
	return (((result + 3) >> 2) << 2);
}

void free(void *ptr)
{
	// Do freeing of memory
}
#if 0
void *realloc(void *ptr, size_t size);

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
#endif

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
					unsigned int v = *args;
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
					args++;
				}
				else if (*format == 'c')
				{
					buffer[0] = *args;
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
#if 0
int vsnprintf(char *str, size_t size, const char *format, va_list ap);

#endif
#define O_RDONLY 0
#define O_WRONLY 1
#define O_CREAT 00100
#define O_TRUNC 001000

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

int open(const char *filename, int flag, int mode)
{
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
		open_mode = O_WRONLY;
	else
	{
		printf("Mode %s should be 'r' or 'w'\n", mode);
		return 0;
	}
	int fh = sys_int80(5, pathname, open_mode, 0);
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
#if 0
FILE *fdopen(int fd, const char *mode);
#endif
int fclose(FILE *stream)
{
	return sys_int80(6, stream->fh, 0, 0);
}
#if 0
int fflush(FILE *stream);
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
#endif

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
#if 0


double ldexp(double x, int exp);

time_t time(time_t *tloc);

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
struct tm *localtime(const time_t *timep);

struct timeval {
	time_t      tv_sec;     /* seconds */
	suseconds_t tv_usec;    /* microseconds */
};
struct timezone {
	int tz_minuteswest;     /* minutes west of Greenwich */
	int tz_dsttime;         /* type of DST correction */
};
int gettimeofday(struct timeval *tv, struct timezone *tz);

int errno;

#define NULL 0

// This is a hack for an enum that is defined inside a struct
const int LINE_MACRO_OUTPUT_FORMAT_GCC = 0;
const int LINE_MACRO_OUTPUT_FORMAT_NONE = 1;
const int LINE_MACRO_OUTPUT_FORMAT_STD = 2;
const int LINE_MACRO_OUTPUT_FORMAT_P10 = 11;

// for tcc_cc.c
#endif
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

#if 0

char *getcwd(char *buf, size_t size);
char *getenv(const char *name);
void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
void va_end(va_list ap);
time_t time(time_t *tloc);

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

int unlink(const char *pathname);


int sscanf(const char *str, const char *format, ...);
int atoi(const char *nptr);
int remove(const char *pathname);
int execvp(const char *file, char * argv[]);

void __init_globals__(void);

#endif