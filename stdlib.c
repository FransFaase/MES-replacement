// Functions defined in stack_c_intro.M1

int sys_int80(int a, int b, int c, int d); // https://faculty.nps.edu/cseagle/assembly/sys_call.html
int sys_malloc(size_t size);



typedef uint32_t size;
typedef uint32_t off_t;
typedef uint32_t time_t;
typedef uint32_t suseconds_t;

typedef struct 
{
	uint32_t fh;
	uint32_t pos;
	uint32_t length;
} FILE;
FILE __sys_stdin = { 0, 0, 0 };
FILE __sys_stdout = { 1, 0, 0 };
FILE __sys_stderr = { 2, 0, 0 };

const FILE *stdin = &__sys_stdin;
const FILE *stdout = &__sys_stdout;
const FILE *stderr = &__sys_stderr;

#if 0
void *memcpy(void *dest, const void *src, size_t n);
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
	while (*src != '\0')
		*dest++ = *src++;
	*dest = '\0';
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
	char buffer[10];
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
			if (*format == '\\')
			{
				format++;
				if (*format == 'n')
					buffer[0] = '\n';
				else if (*format == 'r')
					buffer[0] = '\r';
				else if (*format == 't')
					buffer[0] = '\t';
				else
					buffer[0] = *format;
			}
			else if (*format == '%')
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
						l = 10;
						for (; v != 0; v /= 10)
							buffer[--l] = '0' + v % 10;
						if (*args < 0)
							buffer[--l] = '-';
						s = buffer + l;
						l = 10 - l;
					}
					args++;
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

#if 0
int fprintf(FILE *stream, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	__sys_printf(stream, 0, -1, format, ap);
}
#endif
int printf(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	__sys_printf(stdout, 0, -1, format, ap);
}

#if 0
int sprintf(char *str, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	__sys_printf(0, str, -1, format, ap);
}
int snprintf(char *str, size_t size, const char *format, ...);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);

#endif
#define O_RDONLY 0
#define O_WRONLY 1
#define O_CREAT 00100
#define O_TRUNC 001000

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#if 0
int open(const char *, int, ...);
int close(int fd);

size_t read(int fd, void *buf, size_t count);

#endif
off_t lseek(int fd, off_t offset, int whence)
{
	return sys_int80(19, fd, offset, whence);
}

FILE *fopen(const char *pathname, const char *mode)
{
	int open_mode = 0;
	if (mode[0] == 'a' && mode[1] == '\0')
		open_mode = O_RDONLY;
	else if (mode[0] == 'w' && mode == '\0')
		open_mode = O_WRONLY;
	else
		return 0;
	int fh = sys_int80(5, pathname, open_mode, 0);
	if (fh < 0)
		return 0;
	FILE *f = (FILE*)malloc(sizeof(FILE));
	f->fh = fh;
	f->pos = 0;
	f->length = 0;
	if (open_mode == O_RDONLY)
	{
		f->length = lseek(fh, 0, SEEK_END);
		lseek(fh, 0, SEEK_SET);
	}
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
	return stream->pos >= stream->length;
}
int fgetc(FILE *stream)
{
	if (stream->pos >= stream->length)
		return -1;
	char buffer[0];
	int ret = sys_int80(3, stream->fh, buffer, 1);
	if (ret == 0)
		return -1;
	stream->pos++;
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

int write(int fd, char* buf, unsigned count);
int fileno(FILE *stream);

#define __LINE__ 0
#define __file__ ""
#define __func__ ""

char *getcwd(char *buf, size_t size);
char *getenv(const char *name);
void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
void va_end(va_list ap);
time_t time(time_t *tloc);

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

int unlink(const char *pathname);

void exit(void);

int sscanf(const char *str, const char *format, ...);
int atoi(const char *nptr);
int remove(const char *pathname);
int execvp(const char *file, char * argv[]);

void __init_globals__(void);

#endif