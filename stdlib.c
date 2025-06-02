typedef uint32_t size;
typedef uint32_t off_t;
typedef uint32_t time_t;
typedef uint32_t suseconds_t;

const FILE *stdin = 0;
const FILE *stdout = 1;
const FILE *stderr = 2;

#if 0
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

size_t strlen(const char *s) {}
char *strcpy(char *dest, const char *src)
{

}
char *strncpy(char *dest, const char *src, size_t n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);
int strcmp(const char *s1, const char *s2)
{
	int result;
	for (;;)
	{
		result = *s1 - *s2;
		if (result != 0 || *s1 == 0)
			break;
		s1++;
		s2++;
	}
	return result;
}
int strncmp(const char *s1, const char *s2, size_t n);


long strtol(const char *nptr, char **endptr, int base);
long strtoul(const char *nptr, char **endptr, int base);
long strtoll(const char *nptr, char **endptr, int base);
long strtoull(const char *nptr, char **endptr, int base);

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
#endif

int sys_int80(int a, int b, int c, int d);

int fputc(int c, FILE *stream)
{
	int buffer[1];
	buffer[0] = c;
	return sys_int80(4, stream, buffer, 1);
}
#if 0
int fputs(const char *s, FILE *stream);

int printf(const char *format, ...)
{
}
int fprintf(FILE *stream, const char *format, ...)
{
}
int sprintf(char *str, const char *format, ...) {}
int snprintf(char *str, size_t size, const char *format, ...);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);

#define O_RDONLY 0
#define O_WRONLY 1
#define O_CREAT 00100
#define O_TRUNC 001000

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

int open(const char *, int, ...);
int close(int fd);

size_t read(int fd, void *buf, size_t count);

off_t lseek(int fd, off_t offset, int whence);

FILE *fopen(const char *pathname, const char *mode) {}
FILE *fdopen(int fd, const char *mode);
int fclose(FILE *stream) {}
int fflush(FILE *stream);
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
int feof(FILE *stream) {}
int fgetc(FILE *stream) {}


double ldexp(double x, int exp);


void *malloc(size_t size) {}
void free(void *ptr);
void *realloc(void *ptr, size_t size);

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
void va_start(va_list ap, ...);
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