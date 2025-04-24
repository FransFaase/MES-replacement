// -------- File IO --------

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#ifdef __GNUC__

#include <stdio.h>
#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

char fhgetc(int f)
{
	char ch[1];
	int i = read(f, ch, 1);
	/*
	if (i == 0)
		printf("<EOF>\n");
	else if (' ' <= ch[0] && ch[0] < 127)
		printf("%c", ch[0]);
	else if (ch[0] == '\n')
		printf("\\n\n");
	else
		printf("\\%o", ch[0]);
	*/
	if (i == 0)
		return 0;
	return ch[0];
}

void fhputc(char s, int f)
{
	char buffer[2];
	buffer[0] = s;
	write(f, buffer, 1);
}

//void fhputs(char* s, int f)
//{
//	size_t len = strlen(s);
//	write(f, s, len);
//}

#else

int open(char* name, int flag, int mode)
{
	asm("lea_ebx,[esp+DWORD] %12"
	    "mov_ebx,[ebx]"
	    "lea_ecx,[esp+DWORD] %8"
	    "mov_ecx,[ecx]"
	    "lea_edx,[esp+DWORD] %4"
	    "mov_edx,[edx]"
	    "mov_eax, %5"
	    "int !0x80");
}

int close(int fh)
{
	asm("lea_ebx,[esp+DWORD] %4"
	    "mov_ebx,[ebx]"
	    "mov_eax, %6"
	    "int !0x80");
}

int fhgetc(int fh)
{
	asm("mov_eax, %3"
	    "lea_ebx,[esp+DWORD] %4"
	    "mov_ebx,[ebx]"
	    "push_ebx"
	    "mov_ecx,esp"
	    "mov_edx, %1"
	    "int !0x80"
	    "test_eax,eax"
	    "pop_eax"
	    "jne %FUNCTION_fgetc_Done"
	    "mov_eax, %0"
	    ":FUNCTION_fgetc_Done");
}

void fhputc(char s, int f)
{
	asm("mov_eax, %4"
	    "lea_ebx,[esp+DWORD] %4"
	    "mov_ebx,[ebx]"
	    "lea_ecx,[esp+DWORD] %8"
	    "mov_edx, %1"
	    "int !0x80");
}

#endif

void fhputs(char* s, int f)
{
	while(0 != s[0])
	{
		fhputc(s[0], f);
		s = s + 1;
	}
}

void fhput_int(int i, int fh)
{
	if (i == 0)
		return;
	fhput_int(i / 10, fh);
	fhputc('0' + (i % 10), fh);
}

void fhput_hex(int i, int fh)
{
	int v;
	if (i == 0)
	{
		fhputs("0x", fh);
		return;
	}
	if (i < 0)
	{
		fhputs("-", fh);
		i = 0 - i;
	}
	fhput_hex(i / 16, fh);
	v = i % 16;
	if (v < 10)
		fhputc('0' + v, fh);
	else
		fhputc('a' + v - 10, fh);
}


// ----- Malloc ----

#ifdef __GNUC__

void *_malloc(size_t size)
{
	//printf("malloc %ld ", size);
	//fflush(stdout);
	void* r = malloc(size);
	//printf("%p\n", r);
	return r;
}


#else

#define NULL 0

int brk(void *addr)
{
	asm("mov_eax,[esp+DWORD] %4"
	    "push_eax"
	    "mov_eax, %45"
	    "pop_ebx"
	    "int !0x80");
}

long _malloc_ptr;
long _brk_ptr;

void* _malloc(int size)
{
	if(NULL == _brk_ptr)
	{
		_brk_ptr = brk(0);
		_malloc_ptr = _brk_ptr;
	}

	if(_brk_ptr < _malloc_ptr + size)
	{
		_brk_ptr = brk(_malloc_ptr + size);
		if(-1 == _brk_ptr) return 0;
	}

	long old_malloc = _malloc_ptr;
	_malloc_ptr = _malloc_ptr + size;
	fhputs("malloc ", STDOUT_FILENO);
	fhput_hex(old_malloc, STDOUT_FILENO);
	fhputs("\n", STDOUT_FILENO);
	return old_malloc;
}
#endif

// ----- bool -----

typedef char bool;
#define TRUE 1
#define FALSE 0

// ------ string -------

bool eqstr(char* s, char* t)
{
	while (*s == *t)
	{
		if (*s == 0)
		{
			return TRUE;
		}
		s = s + 1;
		t = t + 1;
	}
	return FALSE;
} 

// ----- Strings ----

void _strcpy(char* trg, char* src)
{
	int i = 0;
	while (*src != 0)
	{
		trg[i] = *src;
		i = i + 1;
		src = src + 1;
	}
	trg[i] = 0;
}

void _strcat(char* trg, char* src)
{
	while (*trg != 0)
	{
		trg = trg + 1;
	}
	_strcpy(trg, src);
}

char* copystr(const char* str)
{
	int len = 0;
	while (str[len] != 0)
	{
		len = len + 1;
	}
	
	//printf("copystr %d %ld\n", len + 1, strlen(str) + 1);
	char* new_str = _malloc(len + 1);
	
	while (len >= 0)
	{
		new_str[len] = str[len];
		len = len - 1;
	}

	return new_str;
}

// Preprocessor

typedef struct char_iterator_s char_iterator_t;
typedef struct char_iterator_s* char_iterator_p;
typedef void (*char_next_p)(char_iterator_p);
struct char_iterator_s
{
	char ch;
	long line;
	long column;
	char* filename;
	char_next_p next;
};


struct file_iterator_s
{
	char_iterator_t base;
	int _f;
};
typedef struct file_iterator_s file_iterator_t;
typedef struct file_iterator_s* file_iterator_p;

void file_iterator_next(char_iterator_p char_it)
{
	file_iterator_p it = (file_iterator_p)char_it;
	//fhputs("file_iterator_next: '", STDOUT_FILENO);
	if (it->base.ch == 0)
	{
		return;
	}
	if (it->base.ch == '\n')
	{
		it->base.line = it->base.line + 1;
		it->base.column = 0;
	}
	it->base.column = it->base.column + 1;
	it->base.ch = fhgetc(it->_f);
	if (it->base.ch == 0)
	{
		if (it->_f >= 0)
		{
			close(it->_f);
			it->_f = -1;
		}
	}
	else if (it->base.ch == '\r')
	{
		file_iterator_next(char_it);
	}
	//fhputc(it->base.ch, STDOUT_FILENO);
	//fhputs("'\n", STDOUT_FILENO);
}

file_iterator_p new_file_iterator(char* fn)
{
	file_iterator_p it = _malloc(sizeof(struct file_iterator_s));
	it->_f = open(fn, 0, 0);
	it->base.ch = '\n';
	if (it->_f < 0)
	{
		it->base.ch = 0;
	}
	it->base.filename = copystr(fn);
	it->base.line = 0;
	it->base.column = 0;
	it->base.next = file_iterator_next;
	file_iterator_next(&it->base);
	return it;
}


struct line_splice_iterator_s
{
	char_iterator_t base;
	char_iterator_p _source_it;
	char _a;
};
typedef struct line_splice_iterator_s* line_splice_iterator_p;

void line_splice_iterator_next(char_iterator_p char_it)
{
	line_splice_iterator_p it = (line_splice_iterator_p)char_it;
	char_next_p _source_it_next; _source_it_next = it->_source_it->next;
	//fhputs("line_splice_iterator_next(\n", STDOUT_FILENO);
	if (it->_a == 0)
	{
		it->base.ch = 0;
		return;
	}
	if (it->_a == ' ' || it->_a == '\t')
	{
		it->base.line = it->_source_it->line;
		it->base.column = it->_source_it->column;
		while (it->_a == ' ' || it->_a == '\t')
		{
			_source_it_next(it->_source_it);
			it->_a = it->_source_it->ch;
		}
		it->base.ch = ' ';
		return;
	}
	it->base.ch = it->_a;
	it->base.filename = it->_source_it->filename;
	it->base.line = it->_source_it->line;
	it->base.column = it->_source_it->column;
	_source_it_next(it->_source_it);
	it->_a = it->_source_it->ch;
	//fhputc(it->_a, STDOUT_FILENO);
	//fhputs("\n\n", STDOUT_FILENO);
	while (it->base.ch == '\\' && it->_a == '\n')
	{
		_source_it_next(it->_source_it);
		it->base.ch = it->_source_it->ch;
		it->base.filename = it->_source_it->filename;
        it->base.line = it->_source_it->line;
        it->base.column = it->_source_it->column;
        _source_it_next(it->_source_it);
        it->_a = it->_source_it->ch;
    }
}

line_splice_iterator_p new_line_splice_iterator(char_iterator_p source_it)
{
	line_splice_iterator_p it = _malloc(sizeof(struct line_splice_iterator_s));
	it->_source_it = source_it;
	it->base.filename = source_it->filename;
	it->base.next = line_splice_iterator_next;
	it->_a = source_it->ch;
	line_splice_iterator_next(&it->base);
	return it;
}



struct comment_strip_iterator_s
{
	char_iterator_t base;
	char_iterator_p _source_it;
	char _a;
	int _state;
};
typedef struct comment_strip_iterator_s* comment_strip_iterator_p;

void comment_strip_iterator_next(char_iterator_p char_it)
{
	comment_strip_iterator_p it = (comment_strip_iterator_p)char_it;
	char_next_p _source_it_next; _source_it_next = it->_source_it->next;
	//fhputs("comment_strip_iterator_next(\n", STDOUT_FILENO);
	//fhput_int(it->_state, STDOUT_FILENO);
	//fhputs("'\n", STDOUT_FILENO);
    switch (it->_state)
    {
        case 0: goto s0;
        case 1: goto s1;
        case 2: goto s2;
        case 3: goto s3;
        case 4: goto s4;
        case 5: goto s5;
        case 6: goto s6;
        case 7: goto s7;
        case 8: goto s8;
        case 9: goto s9;
        default:
    }
    s0:
    //fhputs("s0:\n", STDOUT_FILENO);
    if (it->_a == '\0')
    {
        it->base.ch = '\0';
        return;
    }
    it->base.ch = it->_a;
    it->base.filename = it->_source_it->filename;
    it->base.line = it->_source_it->line;
    it->base.column = it->_source_it->column;
    _source_it_next(it->_source_it);
    it->_a = it->_source_it->ch;
    if (it->base.ch == '/' && (it->_a == '/' || it->_a == '*'))
    {
        if (it->_a == '/')
        {
            while (it->_a != '\0' && it->_a != '\n')
            {
                _source_it_next(it->_source_it);
                it->_a = it->_source_it->ch;
            }
        }
        else
        {
            it->base.ch = ' ';
            it->_state = 1; return; s1:
            _source_it_next(it->_source_it);
            it->base.ch = it->_source_it->ch;
            _source_it_next(it->_source_it);
            it->_a = it->_source_it->ch;
            while (it->base.ch != '\0' && (it->base.ch != '*' || it->_a != '/'))
            {
                it->base.ch = it->_a;
                _source_it_next(it->_source_it);
                it->_a = it->_source_it->ch;
            }
            if (it->base.ch != '\0')
            {
                _source_it_next(it->_source_it);
                it->_a = it->_source_it->ch;
                it->base.line = it->_source_it->line;
                it->base.column = it->_source_it->column;
            }
        }
        it->_state = 0;
        goto s0;
    }
    if (it->base.ch == '"')
    {
        it->_state = 2; return; s2:
        it->base.ch = it->_a;
        it->base.line = it->_source_it->line;
        it->base.column = it->_source_it->column;
        
        while (it->base.ch != '\0' && it->base.ch != '"')
        {
            if (it->base.ch == '\\')
            {
                it->_state = 3; return; s3:
                _source_it_next(it->_source_it);
                it->base.ch = it->_source_it->ch;
                it->base.line = it->_source_it->line;
                it->base.column = it->_source_it->column;
            }
            
            it->_state = 4; return; s4:
            _source_it_next(it->_source_it);
            it->base.ch = it->_source_it->ch;
            it->base.line = it->_source_it->line;
            it->base.column = it->_source_it->column;
        }
        if (it->base.ch == '"')
        {
            it->_state = 5; return; s5:
            _source_it_next(it->_source_it);
            it->_a = it->_source_it->ch;
            it->base.line = it->_source_it->line;
            it->base.column = it->_source_it->column;
        }
        it->_state = 0;
        goto s0;
    }
    if (it->base.ch == '\'')
    {
        it->_state = 6; return; s6:
        it->base.ch = it->_a;
        it->base.line = it->_source_it->line;
        it->base.column = it->_source_it->column;
        if (it->base.ch == '\\')
        {
            it->_state = 7; return; s7:
            _source_it_next(it->_source_it);
            it->base.ch = it->_source_it->ch;
            it->base.line = it->_source_it->line;
            it->base.column = it->_source_it->column;
        }
        it->_state = 8; return; s8:
        _source_it_next(it->_source_it);
        it->base.ch = it->_source_it->ch;
        it->base.line = it->_source_it->line;
        it->base.column = it->_source_it->column;
        it->_state = 0;
        if (it->base.ch == '\'')
        {
            it->_state = 9; return; s9:
            _source_it_next(it->_source_it);
            it->_a = it->_source_it->ch;
            it->base.line = it->_source_it->line;
            it->base.column = it->_source_it->column;
        }
        it->_state = 0;
        goto s0;
    }
	//fhputs("comment_strip_iterator_next)\n", STDOUT_FILENO);
}

comment_strip_iterator_p new_comment_strip_iterator(char_iterator_p source_it)
{
	//fhputs("new_comment_strip_iterator(\n", STDOUT_FILENO);
	comment_strip_iterator_p it = _malloc(sizeof(struct comment_strip_iterator_s));
	it->_source_it = source_it;
	it->base.filename = source_it->filename;
	it->base.next = comment_strip_iterator_next;
	it->_a = source_it->ch;
	it->_state = 0;	
	comment_strip_iterator_next(&it->base);
	//fhputs("new_comment_strip_iterator)\n", STDOUT_FILENO);
	return it;
}


struct include_iterator_s
{
	char_iterator_t base;
	char_iterator_p _source_it;
	char_iterator_p _parent_source_its[10];
	int _nr_parents;
};
typedef struct include_iterator_s* include_iterator_p;

void include_iterator_next(char_iterator_p char_it)
{
	include_iterator_p it = (include_iterator_p)char_it;
	//fhputs("include_iterator_next(\n", STDOUT_FILENO);
	//printf("include_iterator_next %d\n", it->_source_it->ch);
	char_next_p _source_it_next; _source_it_next = it->_source_it->next;
	_source_it_next(it->_source_it);
	it->base.ch = it->_source_it->ch;
	if (it->base.ch == 0)
	{
		//printf("include_iterator_next end %s %d\n", it->base.filename, it->_nr_parents); 
		if (it->_nr_parents == 0)
		{
			it->base.ch = 0;
			return;
		}
		it->base.ch = '\n';
		//free(it->_source_it);
		it->_nr_parents = it->_nr_parents - 1;
		it->_source_it = it->_parent_source_its[it->_nr_parents];
		it->base.filename = it->_source_it->filename;
	}
	it->base.filename = it->_source_it->filename;
	it->base.line = it->_source_it->line;
	it->base.column = it->_source_it->column;
	//printf(it->base.ch < ' ' ? "= %d\n" : "= '%c'\n", it->base.ch);
	//fhputs("include_iterator_next: '", STDOUT_FILENO);
	//fhputc(it->base.ch, STDOUT_FILENO);
	//fhputs("'\n", STDOUT_FILENO);
}			

void include_iterator_add(include_iterator_p it, char_iterator_p include_it)
{
	//printf("include_iterator_add %d\n", it->_nr_parents);
	it->_parent_source_its[it->_nr_parents] = it->_source_it;
	it->_nr_parents = it->_nr_parents + 1;
	it->_source_it = include_it;
	it->base.filename = it->_source_it->filename;
	it->base.line = it->_source_it->line;
	it->base.column = it->_source_it->column;
	it->base.ch = it->_source_it->ch;
	//printf("Next char %d\n", it->base.ch);
}

include_iterator_p new_include_iterator(char_iterator_p include_it)
{
	//fhputs("new_include_iterator(\n", STDOUT_FILENO);
	include_iterator_p it = _malloc(sizeof(struct include_iterator_s));
	it->base.filename = include_it->filename;
	it->base.line = include_it->line;
	it->base.column = include_it->column;
	it->base.ch = include_it->ch;
	it->base.next = include_iterator_next;
	it->_source_it = include_it;
	it->_nr_parents = 0;
	//fhputs("new_include_iterator)\n", STDOUT_FILENO);
	return it;
}


#define TK_D_HASH	 500
#define TK_EQ		 501
#define TK_NE		 502
#define TK_LE		 503
#define TK_GE		 504
#define TK_INC		 505
#define TK_DEC		 506
#define TK_ARROW	 507
#define TK_DPERIOD   508
#define TK_DASHES    509

#define TK_ASS		600
#define TK_MUL_ASS  (600 + '*')
#define TK_DIV_ASS	(600 + '/')
#define TK_MOD_ASS	(600 + '%')
#define TK_ADD_ASS	(600 + '+')
#define TK_SUB_ASS	(600 + '-')
#define TK_SHL_ASS	(600 + '<')
#define TK_SHR_ASS	(600 + '>')

#define TK_DD_OPER	800
#define TK_SHL		(800 + '<')
#define TK_SHR		(800 + '>')
#define TK_AND		(800 + '&')
#define TK_OR		(800 + '|')

#define TK_KEYWORD		1000
#define TK_CASE			1000
#define TK_CHAR			1001
#define TK_CONST		1002
#define TK_DEFAULT		1003
#define TK_DO			1004
#define TK_DOUBLE		1005
#define TK_ELSE			1006
#define TK_ENUM			1007
#define TK_EXTERN		1008
#define TK_FLOAT        1009
#define TK_FOR			1010
#define TK_GOTO			1011
#define TK_IF			1012
#define TK_INLINE		1013
#define TK_INT			1014
#define TK_LONG			1015
#define TK_RETURN       1016
#define TK_SHORT		1017
#define TK_SIZEOF		1018
#define TK_STATIC		1019
#define TK_STRUCT		1020
#define TK_SWITCH		1021
#define TK_THEN			1022
#define TK_TYPEDEF		1023
#define TK_UNION		1024
#define TK_UNSIGNED		1025
#define TK_VOID			1026
#define TK_WHILE		1027
#define TK_H_ELSE		1028
#define TK_H_ELIF		1029
#define TK_H_ENDIF		1030
#define TK_H_DEFINE		1031
#define TK_DEFINED		1032
#define TK_H_IF			1033
#define TK_H_IFDEF		1034
#define TK_H_IFNDEF		1035
#define TK_H_INCLUDE	1036
#define TK_H_UNDEF		1037
#define TK_H_ERROR		1038

typedef struct token_iterator_s token_iterator_t;
typedef struct token_iterator_s* token_iterator_p;
typedef token_iterator_p (*token_next_p)(token_iterator_p, bool);
struct token_iterator_s
{
	int kind;
	char *token;
	int int_value;
	char *filename;
	int line;
	int column;
	token_next_p next;
};

struct tokenizer_s
{
	token_iterator_t base;
	char_iterator_p _char_iterator;
};
typedef struct tokenizer_s *tokenizer_p;

void tokenizer_skip_white_space(tokenizer_p tokenizer, bool skip_nl)
{
	//printf("tokenizer_skip_white_space %p %d\n", tokenizer, skip_nl);
	char_iterator_p it; it = tokenizer->_char_iterator;
	char_next_p it_next = it->next;
	char ch = it->ch;

	while (ch != 0 && ch <= ' ' && (skip_nl || ch != '\n'))
	{
		it_next(it); ch = it->ch;
	}
}

char tokenizer_parse_char_literal(tokenizer_p tokenizer)
{
	char_iterator_p it = tokenizer->_char_iterator;
	char_next_p it_next; it_next = it->next;
	char ch = it->ch;
	it_next(it);
	if (ch != '\\')
	{
		 return ch;
	}
	ch = it->ch;
	it_next(it);
	if (ch == '0' || ch == '1')
	{
		int val = ch - '0';
		ch = it->ch;
		if ('0' <= ch && ch <= '7')
		{
			val = 8 * val + ch - '0';
			it_next(it);
			ch = it->ch;
			if ('0' <= ch && ch <= '7')
			{
				val = 8 * val + ch - '0';
				it_next(it);
			}
			return val;
		}
		else if (val == 0)
		{
			return 0;
		}
		else
		{
			return '1';
		}
	}
	if (ch == 'n')
	{
		ch = '\n';
	}
	else if (ch == 'r')
	{
		ch = '\r';
	}
	return ch;
}

token_iterator_p tokenizer_next(token_iterator_p token_it, bool skip_nl)
{
	tokenizer_p tokenizer = (tokenizer_p)token_it;
	//fhputs("tokenizer_next(\n", STDOUT_FILENO);
	tokenizer_skip_white_space(tokenizer, skip_nl);
	char_iterator_p it = tokenizer->_char_iterator;
	char_next_p it_next; it_next = it->next;
	int i = 0;
	int sign = 1;
	char ch = it->ch;
	tokenizer->base.kind = 0;
	if (ch == 0)
	{
		goto done;
	}
	bool at_start_of_line =    token_it->filename != tokenizer->_char_iterator->filename
							|| token_it->line != tokenizer->_char_iterator->line;
	token_it->filename = it->filename;
	token_it->line = it->line;
	token_it->column = it->column;
	if (ch == '\n')
	{
		token_it->kind = '\n';
	}
	else if (ch == '#' && at_start_of_line)
	{
		token_it->token[i] = '#'; i = i + 1; it_next(it);
		tokenizer_skip_white_space(tokenizer, TRUE);
		ch = it->ch;
		while ('a' <= ch && ch <= 'z')
		{
			token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
		}
		token_it->token[i] = '\0';
		     if (eqstr("#else",    token_it->token)) token_it->kind = TK_H_ELSE;
		else if (eqstr("#elif",    token_it->token)) token_it->kind = TK_H_ELIF;
		else if (eqstr("#endif",   token_it->token)) token_it->kind = TK_H_ENDIF;
		else if (eqstr("#define",  token_it->token)) token_it->kind = TK_H_DEFINE;
		else if (eqstr("#if",      token_it->token)) token_it->kind = TK_H_IF;
		else if (eqstr("#ifdef",   token_it->token)) token_it->kind = TK_H_IFDEF;
		else if (eqstr("#ifndef",  token_it->token)) token_it->kind = TK_H_IFNDEF;
		else if (eqstr("#include", token_it->token)) token_it->kind = TK_H_INCLUDE;
		else if (eqstr("#undef",   token_it->token)) token_it->kind = TK_H_UNDEF;
		else if (eqstr("#error",   token_it->token)) token_it->kind = TK_H_ERROR;
	}
	else if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_')
	{
		token_it->kind = 'i';
		token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
		while (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ch == '_')
		{
			token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
		}
		token_it->token[i] = '\0';
		     if (eqstr("case",     token_it->token)) token_it->kind = TK_CASE;
		else if (eqstr("char",     token_it->token)) token_it->kind = TK_CHAR;
		else if (eqstr("const",    token_it->token)) token_it->kind = TK_CONST;
		else if (eqstr("default",  token_it->token)) token_it->kind = TK_DEFAULT;
		else if (eqstr("defined",  token_it->token)) token_it->kind = TK_DEFINED;
		else if (eqstr("do",       token_it->token)) token_it->kind = TK_DO;
		else if (eqstr("double",   token_it->token)) token_it->kind = TK_DOUBLE;
		else if (eqstr("else",     token_it->token)) token_it->kind = TK_ELSE;
		else if (eqstr("enum",     token_it->token)) token_it->kind = TK_ENUM;
		else if (eqstr("extern",   token_it->token)) token_it->kind = TK_EXTERN;
		else if (eqstr("float",    token_it->token)) token_it->kind = TK_FLOAT;
		else if (eqstr("for",      token_it->token)) token_it->kind = TK_FOR;
		else if (eqstr("goto",     token_it->token)) token_it->kind = TK_GOTO;
		else if (eqstr("if",       token_it->token)) token_it->kind = TK_IF;
		else if (eqstr("inline",   token_it->token)) token_it->kind = TK_INLINE;
		else if (eqstr("int",      token_it->token)) token_it->kind = TK_INT;
		else if (eqstr("long",     token_it->token)) token_it->kind = TK_LONG;
		else if (eqstr("return",   token_it->token)) token_it->kind = TK_RETURN;
		else if (eqstr("short",    token_it->token)) token_it->kind = TK_SHORT;
		else if (eqstr("sizeof",   token_it->token)) token_it->kind = TK_SIZEOF;
		else if (eqstr("static",   token_it->token)) token_it->kind = TK_STATIC;
		else if (eqstr("struct",   token_it->token)) token_it->kind = TK_STRUCT;
		else if (eqstr("switch",   token_it->token)) token_it->kind = TK_SWITCH;
		else if (eqstr("then",     token_it->token)) token_it->kind = TK_THEN;
		else if (eqstr("typedef",  token_it->token)) token_it->kind = TK_TYPEDEF;
		else if (eqstr("union",    token_it->token)) token_it->kind = TK_UNION;
		else if (eqstr("unsigned", token_it->token)) token_it->kind = TK_UNSIGNED;
		else if (eqstr("void",     token_it->token)) token_it->kind = TK_VOID;
		else if (eqstr("while",    token_it->token)) token_it->kind = TK_WHILE;
	}
	else if (('0' <= ch && ch <= '9') || ch == '-')
	{
		if (ch == '-')
		{
			token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
			if (ch == '-')
			{
				token_it->kind = TK_DEC;
				token_it->token[i] = ch; i = i + 1; it_next(it);
				goto done;
			}
			else if (ch == '=')
			{
				token_it->kind = TK_SUB_ASS;
				token_it->token[i] = ch; i = i + 1; it_next(it);
				goto done;
			}
			else if (ch == '>')
			{
				token_it->kind = TK_ARROW;
				token_it->token[i] = ch; i = i + 1; it_next(it);
				goto done;
			}
			else if (ch < '0' || '9' < ch)
			{
				token_it->kind = '-';
				goto done;
			}
			sign = -1;
		}			
		
		token_it->kind = '0';
		token_it->int_value = 0;
		if (ch == '0')
		{
			token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
			if (ch == 'x')
			{
				token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
				while (1)
				{
					if ('0' <= ch && ch <= '9')
						token_it->int_value = 16 * token_it->int_value + ch - '0';
					else if ('a' <= ch && ch <= 'f')
						token_it->int_value = 16 * token_it->int_value + ch - 'a' + 10;
					else if ('A' <= ch && ch <= 'F')
						token_it->int_value = 16 * token_it->int_value + ch - 'A' + 10;
					else
						break;
					token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
				}
			}
			else
			{
				while ('0' <= ch && ch <= '7')
				{
					token_it->int_value = 8 * token_it->int_value + ch - '0';
					token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
				}
			}
		}
		else
		{
			while ('0' <= ch && ch <= '9')
			{
				token_it->int_value = 10 * token_it->int_value + ch - '0';
				token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
			}
		}
		if (ch == 'U')
		{
			token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
		}
		while (ch == 'L')
		{
			token_it->token[i] = ch; i = i + 1; it_next(it); ch = it->ch;
		}
		token_it->int_value *= sign;
	}
	else if (ch == '\'')
	{
		token_it->kind = ch;
		it_next(it);
		token_it->token[0] = tokenizer_parse_char_literal(tokenizer);
		i = 1;
		if (it->ch == '\'')
		{
			it_next(it);
		}
		token_it->int_value = i;
	}
	else if (ch == '"')
	{
		token_it->kind = ch; it_next(it); ch = it->ch;
		while (ch != '\0' && ch != '\n' && ch != '"')
		{
			token_it->token[i] = tokenizer_parse_char_literal(tokenizer);
			i = i + 1;
			ch = it->ch;
		}
		if (ch == '"')
		{
			it_next(it);
		}
		token_it->int_value = i;
	}
	else
	{
		token_it->kind = ch;
		token_it->token[0] = ch;
		i = 1;
		if (ch == '#')
		{
			it_next(it);
			if (it->ch == '#')
			{
				it_next(it);
				token_it->kind = TK_D_HASH;
				token_it->token[1] = '#';
				i = 2;
			}
		}
		else if (ch == '=')
		{
			it_next(it);
			if (it->ch == '=')
			{
				it_next(it);
				token_it->kind = TK_EQ;
				token_it->token[1] = '=';
				i = 2;
			}
		}
		else if (ch == '*' || ch == '/' || ch == '%' || ch == '^')
		{
			it_next(it);
			if (it->ch == '=')
			{
				it_next(it);
				token_it->kind = TK_ASS + ch;
				token_it->token[1] = '=';
				i = 2;
			}
		}
		else if (ch == '+' || ch == '-')
		{
			it_next(it);
			ch = it->ch;
			if (ch == token_it->token[0] || ch == '=')
			{
				it_next(it);
				token_it->token[1] = ch;
				if (ch == '+')
				{
					token_it->kind = TK_INC;
				}
				else if (ch == '-')
				{
					token_it->kind = TK_DEC;
				}
				else
				{
					token_it->kind = TK_ASS + token_it->token[0];
				}
				i = 2;
			}
		}
		else if (ch == '<' || ch == '>' || ch == '|' || ch == '&')
		{
			it_next(it);
			if (it->ch == ch)
			{
				it_next(it);
				token_it->kind = TK_DD_OPER + ch;
				token_it->token[1] = ch;
				i = 2;
			}
			if (it->ch == '=' && (ch == '<' || ch == '>')) 
			{
				it_next(it);
				if (i == 2)
				{
					token_it->kind = TK_ASS + ch;
				}
				else if (ch == '<')
				{
					token_it->kind = TK_LE;
				}
				else
				{
					token_it->kind = TK_GE;
				}
				token_it->token[i] = '=';
				i = i + 1;
			}
		}
		else if (ch == '!')
		{
			it_next(it);
			if (it->ch == '=')
			{
				it_next(it);
				token_it->kind = TK_NE;
				token_it->token[1] = '=';
				i = 2;
			}
		}
		else if (ch == '.')
		{
			it_next(it);
			if (it->ch == '.')
			{
				it_next(it);
				token_it->kind = TK_DPERIOD;
				token_it->token[1] = '.';
				i = 2;
				if (it->ch == '.')
				{
					it_next(it);
					token_it->kind = TK_DASHES;
					token_it->token[2] = '.';
					i = 3;
				}
			}
		}
		else
		{
			it_next(it);
		}
	}
	done: token_it->token[i] = '\0';
	printf("tokenizer_next %d '%s'\n", token_it->kind, token_it->token);
	//fhputs("tokenizer_next) ", STDOUT_FILENO);
	//fhput_int(token_it->kind, STDOUT_FILENO);
	//fhputs(" '", STDOUT_FILENO);
	//fhputs(token_it->token, STDOUT_FILENO);
	//fhputs("'\n", STDOUT_FILENO);
	//printf("tokenizer result %d '%s' %d\n", token_it->kind, token_it->token, token_it->int_value);
	return token_it;
}

tokenizer_p new_tokenizer(char_iterator_p char_iterator)
{
	//fhputs("new_tokenizer(\n", STDOUT_FILENO);
	tokenizer_p tokenizer = _malloc(sizeof(struct tokenizer_s));
	tokenizer->_char_iterator = char_iterator;
	tokenizer->base.token = _malloc(6000);
	tokenizer->base.filename = NULL;
	tokenizer->base.next = tokenizer_next;
	//fhputs("new_tokenizer)\n", STDOUT_FILENO);
	return tokenizer;
}

struct tokens_s
{
	int kind;
	char *token;
	int int_value;
	char *filename;
	int line;
	int column;
	struct tokens_s* next;
};
typedef struct tokens_s* tokens_p;

tokens_p new_token(int kind)
{
	tokens_p token = _malloc(sizeof(struct tokens_s));
	token->kind = kind;
	token->token = 0;
	token->int_value = 0;
	token->filename = "<env>";
	token->line = 0;
	token->column = 0;
	token->next = 0;
	return token;
}
tokens_p new_int_token(int int_value)
{
	tokens_p token = new_token('0');
	token->int_value = int_value;
	return token;
}
tokens_p new_str_token(char *str)
{
	tokens_p token = new_token('i');
	token->token = copystr(str);
	return token;
}
tokens_p new_token_from_it(token_iterator_p it)
{
	tokens_p token = new_token(it->kind);
	token->token = copystr(it->token);
	token->int_value = it->int_value;
	token->filename = it->filename;
	token->line = it->line;
	token->column = it->column;
	return token;
}
	

struct env_s {
	char* name;
	int nr_args;
	char* args[10];
	tokens_p tokens;
	struct env_s* next;
};
typedef struct env_s env_t;
typedef struct env_s *env_p;

env_p environment = NULL;

env_p new_env(char* name, char* value)
{
	env_p env = _malloc(sizeof(struct env_s));
	env->name = name;
	env->nr_args = 0;
	env->tokens = NULL;
	env->next = NULL;
	return env;
}

env_p get_env(char* name, bool create)
{
	env_p prev_env = NULL;
	env_p env = environment;
	while (env != NULL)
	{
		if (eqstr(env->name, name))
		{
			//printf("get_env(%s): found\n", name);
			return env;
		}
		prev_env = env;
		env = env->next;
	}
	if (!create)
	{
		//printf("get_env(%s): not found\n", name);
		return NULL;
	}
	//printf("get_env(%s): create\n", name);
	env = _malloc(sizeof(struct env_s));
	env->name = copystr(name);
	env->nr_args = 0;
	env->tokens = NULL;
	env->next = NULL;
	if (prev_env == NULL)
	{
		environment = env;
	}
	else
	{
		prev_env->next = env;
	}
	return env;
}

void del_env(char* name)
{
	env_p prev_env = NULL;
	env_p env = environment;
	while (env != 0)
	{
		if (eqstr(env->name, name))
		{
			if (prev_env == NULL)
			{
				environment = environment->next;
			}
			else
			{
				prev_env->next = env->next;
			}
			return;
		}
		prev_env = env;
		env = env->next;
	}
}

#ifdef __GNUC__
typedef struct conditional_iterator_s* conditional_iterator_p;
typedef int (*parse_expr_function_p)(conditional_iterator_p it);
#else
#define parse_expr_function_p FUNCTION
#endif

struct conditional_iterator_s
{
	token_iterator_t base;
	include_iterator_p _source_it;
	token_iterator_p _token_it;
	parse_expr_function_p _parse_or_expr;
	
	bool _skip_level;
	bool _if_done[40];
	int _if_level;
};
#ifndef __GNUC__
typedef struct conditional_iterator_s* conditional_iterator_p;
#endif

//FUNCTION conditional_iterator_rec_parse_or_expr;

int conditional_iterator_parse_primary(conditional_iterator_p it)
{
	token_next_p token_it_next = it->_token_it->next;
	int result = 0;
	if (it->_token_it->kind == '(')
	{
		token_it_next(it->_token_it, FALSE);
		parse_expr_function_p parse_or_expr = it->_parse_or_expr;
		result = parse_or_expr(it);
		if (it->_token_it->kind == ')')
		{
			token_it_next(it->_token_it, FALSE);
		}
	}
	else if (it->_token_it->kind == TK_DEFINED)
	{
		token_it_next(it->_token_it, FALSE);
		if (it->_token_it->kind == '(')
		{
			token_it_next(it->_token_it, FALSE);
			//printf("defined(%s) = %d\n", it->_token_it->token, get_env(it->_token_it->token, FALSE) != NULL);
			if (get_env(it->_token_it->token, FALSE) != NULL)
			{
				result = 1;
			}
			token_it_next(it->_token_it, FALSE);
			if (it->_token_it->kind == ')')
			{
				token_it_next(it->_token_it, FALSE);
			}
		}
		else
		{
			if (get_env(it->_token_it->token, FALSE) != NULL)
			{
				result = 1;
			}
			token_it_next(it->_token_it, FALSE);
		}
	}
	else if (it->_token_it->kind == '0')
	{
		result = it->_token_it->int_value;
		token_it_next(it->_token_it, FALSE);
	}
	else if (it->_token_it->kind == 'i')
	{
		env_p env = get_env(it->_token_it->token, FALSE);
		if (env != NULL && env->tokens != NULL && env->tokens->kind == '0')
		{
			result = env->tokens->int_value;
		}
		token_it_next(it->_token_it, FALSE);
	}
	return result;
}

int conditional_iterator_parse_unary_expr(conditional_iterator_p it)
{
	token_next_p token_it_next = it->_token_it->next;
	if (it->_token_it->kind == '!')
	{
		token_it_next(it->_token_it, FALSE);
		int result = conditional_iterator_parse_primary(it);
		//printf("xx ! %d = %d\n", result, !result);
		return !result; //!conditional_iterator_parse_primary(it);
	}
	return conditional_iterator_parse_primary(it);
}

int conditional_iterator_parse_compare_expr(conditional_iterator_p it)
{
	token_next_p token_it_next = it->_token_it->next;
	int value = conditional_iterator_parse_unary_expr(it);
	if (it->_token_it->kind == TK_EQ)
	{
		token_it_next(it->_token_it, FALSE);
		return value == conditional_iterator_parse_unary_expr(it);
	}
	if (it->_token_it->kind == TK_NE)
	{
		token_it_next(it->_token_it, FALSE);
		return value != conditional_iterator_parse_unary_expr(it);
	}
	return value;
}

int conditional_iterator_parse_and_expr(conditional_iterator_p it)
{
	token_next_p token_it_next = it->_token_it->next;
	int value = conditional_iterator_parse_compare_expr(it);
	while (it->_token_it->kind == TK_AND)
	{
		token_it_next(it->_token_it, FALSE);
		int value2 = conditional_iterator_parse_compare_expr(it);
		//printf("xx %d && %d = ", value, value2);
		value = value && value2; //conditional_iterator_parse_primary(it);
		//printf("%d\n", value);
	}
	return value;
}

int conditional_iterator_parse_or_expr(conditional_iterator_p it)
{
	token_next_p token_it_next = it->_token_it->next;
	int value = conditional_iterator_parse_and_expr(it);
	while (it->_token_it->kind == TK_OR)
	{
		token_it_next(it->_token_it, FALSE);
		int value2 = conditional_iterator_parse_and_expr(it);
		value = value || value2;
	}
	return value;
}

char *include_path = 0;

token_iterator_p conditional_iterator_next(token_iterator_p token_it, bool dummy)
{
	conditional_iterator_p it = (conditional_iterator_p)token_it;
	//printf("conditional_iterator_next\n");
	token_next_p token_it_next = it->_token_it->next;
	int kind;
	int value = 0;
	tokens_p prev_token;
	env_p env;
	tokens_p token;
	file_iterator_p input_it;
	line_splice_iterator_p splice_it;
	comment_strip_iterator_p comment_it;
	
	token_it_next(it->_token_it, TRUE);
	while (TRUE)
	{
		if (it->_token_it->token[0] == '#')
		{
			//printf("%s.%d %s (%d)\n", it->_token_it->filename, it->_token_it->line, it->_token_it->token, it->_skip_level);
		}
		kind = it->_token_it->kind;
		if (it->_skip_level > 0)
		{
			if (kind == TK_H_IF || kind == TK_H_IFDEF || kind == TK_H_IFNDEF)
			{
				it->_skip_level = it->_skip_level + 1;
			}
			else if (kind == TK_H_ENDIF)
			{
				it->_skip_level = it->_skip_level - 1;
				if (it->_skip_level == 0)
				{
					it->_if_level = it->_if_level - 1;
				}
			}
			else if (kind == TK_H_ELSE)
			{
				if (it->_skip_level == 1)
				{
					if (it->_if_done[it->_if_level] == FALSE)
					{
						it->_skip_level = 0;
					}
				}
			}
			else if (kind == TK_H_ELIF)
			{
				token_it_next(it->_token_it, FALSE);
				value = conditional_iterator_parse_or_expr(it);
				if (it->_skip_level == 1)
				{
					if (it->_if_done[it->_if_level] == FALSE)
					{
						token_it_next(it->_token_it, FALSE);
						if (value)
						{
							it->_skip_level = 0;
							it->_if_done[it->_if_level] = TRUE;
						}
					}
				}
			}
			token_it_next(it->_token_it, TRUE);
		}
		else if (kind == TK_H_IFDEF || kind == TK_H_IFNDEF)
		{
			token_it_next(it->_token_it, TRUE);
			it->_if_level = it->_if_level + 1;
			if (it->_token_it->kind == 'i')
			{
				//int defined = value = get_env(it->_token_it->token, FALSE) != 0;
				//printf("for %s is defined %d\n", it->_token_it->token, defined);
				if ((get_env(it->_token_it->token, FALSE) != 0) == (kind == TK_H_IFDEF))
				{
					it->_if_done[it->_if_level] = TRUE;
				}
				else
				{
					it->_if_done[it->_if_level] = FALSE;
					it->_skip_level = 1;
				}
				token_it_next(it->_token_it, TRUE);
			}
		}
		else if (kind == TK_H_IF)
		{
			token_it_next(it->_token_it, TRUE);
			it->_if_level = it->_if_level + 1;
			if (conditional_iterator_parse_or_expr(it))
			{
				it->_if_done[it->_if_level] = TRUE;
			}
			else
			{
				it->_if_done[it->_if_level] = FALSE;
				it->_skip_level = 1;
			}
			token_it_next(it->_token_it, TRUE);
		}
		else if (kind == TK_H_ELSE || kind == TK_H_ELIF)
		{
			it->_skip_level = 1;
			while (it->_token_it->kind == '\n')
			{
				token_it_next(it->_token_it, FALSE);
			}
			token_it_next(it->_token_it, TRUE);
		}
		else if (kind == TK_H_ENDIF)
		{	
			it->_if_level = it->_if_level - 1;
			token_it_next(it->_token_it, TRUE);
		}
		else if (kind == TK_H_DEFINE)
		{
			prev_token = 0;
			token_it_next(it->_token_it, FALSE);
			env = get_env(it->_token_it->token, TRUE);
			//printf("%s is now defined\n", env->name);
			if (it->_source_it->base.ch == '(')
			{
				//printf("Arguments");
				token_it_next(it->_token_it, FALSE);
				while (TRUE)
				{
					token_it_next(it->_token_it, FALSE);
					env->args[env->nr_args] = copystr(it->_token_it->token);
					env->nr_args = env->nr_args + 1;
					token_it_next(it->_token_it, FALSE);
					if (it->_token_it->kind != ',')
					{
						break;
					}
				}
				if (it->_token_it->kind == ')')
				{
					token_it_next(it->_token_it, FALSE);
				}
			}
			else
			{
				token_it_next(it->_token_it, FALSE);
			}
			while (it->_token_it->kind != '\n')
			{
				token = new_token_from_it(it->_token_it);
				if (prev_token == NULL)
				{
					env->tokens = token;
				}
				else
				{
					prev_token->next = token;
				}
				prev_token = token;
				token_it_next(it->_token_it, FALSE);
			}
			token_it_next(it->_token_it, TRUE);
			
			//printf("define of %s (", env->name);
			//for (int i = 0; i < env->nr_args; i++)
			//	printf("%s ", env->args[i]);
			//printf(") ");
			//for (tokens_p t = env->tokens; t != 0; t = t->next)
			//{
			//	printf(" %d:%s", t->kind, t->token);
			//}
			//printf("\n");
		}
		else if (kind == TK_H_UNDEF)
		{
			token_it_next(it->_token_it, TRUE);
			del_env(it->_token_it->token);
			token_it_next(it->_token_it, TRUE);
		}
		else if (kind == TK_H_INCLUDE)
		{
			token_it_next(it->_token_it, TRUE);
			if (it->_token_it->kind == '"')
			{
				_strcpy(include_path, "tcc_sources/");
				_strcat(include_path, it->_token_it->token);
				while (it->_token_it->kind != '\n')
				{
					token_it_next(it->_token_it, FALSE);
				}
				//printf("INCLUDE '%s'\n", include_path);
				input_it = new_file_iterator(include_path);
				if (input_it->base.ch != 0)
				{
					splice_it = new_line_splice_iterator(&input_it->base);
					comment_it = new_comment_strip_iterator(&splice_it->base);
					include_iterator_add(it->_source_it, &comment_it->base);
					token_it_next(it->_token_it, TRUE);
				}
				else
				{
					token_it_next(it->_token_it, TRUE);
				}
			}
			else
			{
				while (it->_token_it->kind != '\n')
				{
					token_it_next(it->_token_it, FALSE);
				}
				token_it_next(it->_token_it, TRUE);
			}
		}
		else if (kind == TK_H_ERROR)
		{
			it->base.kind = it->_token_it->kind;
			it->base.token = it->_token_it->token;
			it->base.int_value = it->_token_it->int_value;
			it->base.filename = it->_token_it->filename;
			it->base.line  = it->_token_it->line;
			it->base.column = it->_token_it->column;
			return token_it;
		}
		else
		{
			it->base.kind = it->_token_it->kind;
			it->base.token = it->_token_it->token;
			it->base.int_value = it->_token_it->int_value;
			it->base.filename = it->_token_it->filename;
			it->base.line  = it->_token_it->line;
			it->base.column = it->_token_it->column;
			return token_it;
		}
	}
	return token_it;
}	

conditional_iterator_p new_conditional_iterator(include_iterator_p source_it, token_iterator_p token_it)
{
	//fhputs("new_conditional_iterator(\n", STDOUT_FILENO);
	//printf("new_conditional_iterator\n");
	//token_next_p token_it_next = token_it->next;
	conditional_iterator_p it = _malloc(sizeof(struct conditional_iterator_s));
	it->_source_it = source_it;
	it->_token_it = token_it;
	it->_parse_or_expr = conditional_iterator_parse_or_expr;
	it->_skip_level = 0;
	it->_if_level = 0;
	it->base.next = conditional_iterator_next;
	if (include_path == 0)
	{
		include_path = _malloc(100);
	}
	//fhputs("new_conditional_iterator)\n", STDOUT_FILENO);
	return it;
}

#define APPENDED_TOKEN_LEN 50

typedef struct expand_macro_iterator_s* expand_macro_iterator_p;
struct expand_macro_iterator_s
{
	token_iterator_t base;
	tokens_p param_tokens;
	tokens_p tokens;
	token_iterator_p _rest_it;
	bool stringify;
	char appended_token[APPENDED_TOKEN_LEN];
	env_p _macro;
	tokens_p args[10];
};

token_iterator_p expand_macro_iterator_next(token_iterator_p token_it, bool dummy)
{
	expand_macro_iterator_p it = (expand_macro_iterator_p)token_it;
	if (it->param_tokens != 0)
	{
		token_it->kind = it->stringify ? '"' : it->param_tokens->kind;
		token_it->token = it->param_tokens->token;
		token_it->int_value = it->stringify ? strlen(token_it->token) : it->param_tokens->int_value;
		token_it->filename = it->param_tokens->filename;
		token_it->line = it->param_tokens->line;
		token_it->column = it->param_tokens->column;
		printf("token from arg %d %s\n", token_it->kind, token_it->token == 0 ? "?" : token_it->token);
		it->param_tokens = it->param_tokens->next;
		return token_it;
	}
	
	for (;;)
	{
		tokens_p token = it->tokens;
		if (token == 0)
		{
			token_iterator_p rest_it = it->_rest_it;
			free(it);
			return rest_it->next(rest_it, dummy);
		}
		
		printf("token from macro %d %s\n", token->kind, token->token == 0 ? "?" : token->token);
		it->tokens = token->next;
		it->stringify = FALSE;
		
		if (token->next != NULL && token->next->kind == TK_D_HASH)
		{
			token_it->kind = 'i';
			token_it->filename = token->filename;
			token_it->line = token->line;
			token_it->column = token->column;
			int p = 0;
			for (;;)
			{
				char *s = token->token;
				if (token->kind == 'i')
				{
					int nr_args = it->_macro->nr_args;
					int i = 0;
					for (; i < nr_args; i++)
						if (strcmp(token->token, it->_macro->args[i]) == 0)
							break;
					if (i < nr_args)
					{
						tokens_p tokens = it->args[i];
						if (tokens == NULL || tokens->next != NULL)
							printf("ERROR: append arg not one value\n");
						else
							s = tokens->token;
					}
				}
				for (; *s != '\0'; s++)
					if (p < APPENDED_TOKEN_LEN - 1)
						it->appended_token[p++] = *s;
				token = token->next;
				if (token == NULL || token->kind != TK_D_HASH)
					break;
				token = token->next;
			}
			it->tokens = token;
			it->appended_token[p] = '\0';
			printf("INFO: Appended token '%s'\n", it->appended_token);
			token_it->token = it->appended_token;
			token_it->int_value = p;
			
			return token_it;
		}
		else
		{
			if (token->kind == '#')
			{
				it->stringify = TRUE;
				it->tokens = token = token->next;
				if (it->tokens == 0)
					return it->_rest_it->next(it->_rest_it, dummy);
			}
			
			if (token->kind == 'i')
			{
				int nr_args = it->_macro->nr_args;
				int i = 0;
				for (; i < nr_args; i++)
					if (strcmp(token->token, it->_macro->args[i]) == 0)
						break;
				if (i < nr_args)
				{
					tokens_p tokens = it->args[i];
					if (tokens == 0)
						continue;
					token_it->kind = tokens->kind;
					token_it->token = tokens->token;
					token_it->int_value = tokens->int_value;
					token_it->filename = tokens->filename;
					token_it->line = tokens->line;
					token_it->column = token->column;
					it->param_tokens = tokens->next;
					return token_it;
				}
			}
			token_it->kind = it->stringify ? '"' : token->kind;
			token_it->token = token->token;
			token_it->int_value = token->int_value;
			token_it->filename = token->filename;
			token_it->line = token->line;
			token_it->column = token->column;
			
			return token_it;
		}
	}
}

token_iterator_p new_exapnd_macro_iterator(env_p macro, tokens_p *args, token_iterator_p rest_it)
{
	expand_macro_iterator_p it = _malloc(sizeof(struct expand_macro_iterator_s));
	it->base.next = expand_macro_iterator_next;
	it->param_tokens = NULL;
	it->tokens = macro->tokens;
	it->_rest_it = rest_it;
	it->_macro = macro;
	for (int arg_nr = 0; arg_nr < macro->nr_args; arg_nr++)
		it->args[arg_nr] = args[arg_nr];
	return expand_macro_iterator_next(&it->base, FALSE);
}

typedef struct expand_iterator_s* expand_iterator_p;
struct expand_iterator_s
{
	token_iterator_t base;
	token_iterator_p _source_it;
};

token_iterator_p expand_iterator_next(token_iterator_p token_it, bool dummy)
{
	expand_iterator_p it = (expand_iterator_p)token_it;
	token_iterator_p source_it = it->_source_it;

	source_it = source_it->next(source_it, dummy);
	
	for (bool go = TRUE; go;)
	{
		go = FALSE;
		if (source_it->kind == 'i')
		{
			env_p macro = get_env(source_it->token, FALSE);
			if (macro != NULL)
			{
				go = TRUE;
				printf("Expand token %s %d: ", source_it->token, macro->nr_args);
				tokens_p args[10];
				int nr_args = 0;
				if (macro->nr_args > 0)
				{
					source_it = source_it->next(source_it, dummy);
					if (source_it->kind != '(')
					{
						printf("ERROR: No arguments for %s when parameters (%d) are expected\n", macro->name, macro->nr_args);
						continue;
					}
					do
					{
						source_it = source_it->next(source_it, dummy);
						args[nr_args] = NULL;
						tokens_p *ref_tokens = &args[nr_args];
						nr_args++;
						char stack[20];
						int stack_depth = 0;
						while (stack_depth > 0 || (source_it->kind != 0 && source_it->kind != ',' && source_it->kind != ')'))
						{
							*ref_tokens = new_token_from_it(source_it);
							ref_tokens = &(*ref_tokens)->next;
							if (source_it->kind == '(')
								stack[stack_depth++] = ')';
							else if (source_it->kind == '{')
								stack[stack_depth++] = '}';
							else if (source_it->kind == '[')
								stack[stack_depth++] = ']';
							else if (stack_depth > 0 && source_it->kind == stack[stack_depth - 1])
								stack_depth--;
							source_it = source_it->next(source_it, dummy);
						}
					}
					while (source_it->kind == ',');
				}
				printf(" %d\n", nr_args);
				if (macro->nr_args != nr_args)
				{
					printf("ERROR: Number arguments (%d) for %s does not match parameters (%d)\n", nr_args, macro->name, macro->nr_args);
					source_it = source_it->next(source_it, dummy);
				}
				else if (macro->tokens == NULL)
					source_it = source_it->next(source_it, dummy);
				else						
					source_it = new_exapnd_macro_iterator(macro, args, source_it);
			}
		}
		it->_source_it = source_it;
	}
	
	token_it->kind = source_it->kind;
	token_it->token = source_it->token;
	token_it->int_value = source_it->int_value;
	token_it->filename = source_it->filename;
	token_it->line = source_it->line;
	token_it->column = source_it->column;
			
	return token_it;
}

expand_iterator_p new_expand_iterator(token_iterator_p source_it)
{
	expand_iterator_p it = _malloc(sizeof(struct expand_iterator_s));
	it->_source_it = source_it;
	it->base.next = expand_iterator_next;
	return it;
}
/*
struct tokens_iterator_s 
{
	token_iterator_t base;
	token_iterator_p _next;
};
typedef struct tokens_iterator_s *tokens_iterator_p;

tokens_iterator_p alloced_tokens_iterators = NULL;

tokens_iterator_p tokens_iterator_next(tokens_iterator_p it, bool skip_nl)
{
	tokens_iterator_p next = it->_next;
	it->_next = alloced_tokens_iterators;
	alloced_tokens_iterators = it;
	return next;	
}

tokens_iterator_p new_tokens_iterator(token_iterator_p it, bool skip_nl)
{
	tokens_iterator_p new_it;
	if (alloced_tokens_iterators != NULL)
	{
		new_it = alloced_tokens_iterators;
		alloced_tokens_iterators = new_it->_next;
	}
	else
	{
		new_it = _malloc(sizeof(struct token_s));
	}
	new_it->base.kind = it->base.kind;
	new_it->base.token = it->base.token;
	new_it->base.int_value = it->base.int_value;
	new_it->base.filenam = it->base.filenam;
	new_it->base.line = it->base.line;
	new_it->base.column = it->base.column;
	new_it->base.next = new_token_iterator;
	new_it->_next = NULL;
	return new_it;
}
*/

token_iterator_p token_it = NULL;

void output_preprocessor(const char *filename)
{
	FILE *fout = fopen(filename, "w");
	if (fout == NULL)
		return;
	int fouth = fileno(fout);
	
	token_it = token_it->next(token_it, TRUE);
	
	char *prev_file = 0;
	int prev_line = 0;
	while (token_it->kind != 0)
	{
		if (token_it->filename != prev_file || token_it->line != prev_line)
		{
			fhputs("\n", fouth);
			fhputs(token_it->filename, fouth);
			fhputs(": ", fouth);
			fhput_int(token_it->line, fouth);
			for (int i = 0; i < token_it->column; i++)
				fhputc(' ', fouth);
			prev_file = token_it->filename;
			prev_line = token_it->line;
		}
		fhputc(' ', fouth);
		if (token_it->kind == 'i')
		{
			fhputs(token_it->token, fouth);
		}
		else if (token_it->kind == '"' || token_it->kind == '\'')
		{
			char strsep = token_it->kind;
			fhputc(strsep, fouth);
			for (int i = 0; i < token_it->int_value ; i = i + 1)
			{
				char ch = token_it->token[i];
				if (ch == '\n')
					fhputs("\\n", fouth);
				else if (ch == '\r')
					fhputs("\\r", fouth);
				else if (ch == '\0')
					fhputs("\\0", fouth);
				else if (' ' <= ch && ch < 127)
					fhputc(ch, fouth);
				else
				{
					fhputs("\\", fouth);
					fhputc(ch, fouth);
				}
			}
			fhputc(strsep, fouth);
		}
		else if (token_it->kind == '0')
		{
			if (token_it->int_value == 0)
				fhputc('0', fouth);
			else
				fhput_int(token_it->int_value, fouth);
		}
		else if (token_it->kind < 127)
		{
			fhputc(token_it->kind, fouth);
		}
		else if (token_it->kind >= TK_KEYWORD)
			fhputs(token_it->token, fouth);
		else if (token_it->kind == TK_D_HASH) 	fhputs("##", fouth);
		else if (token_it->kind == TK_EQ) 		fhputs("==", fouth);
		else if (token_it->kind == TK_NE)		fhputs("!=", fouth);
		else if (token_it->kind == TK_LE)		fhputs("<=", fouth);
		else if (token_it->kind == TK_GE)		fhputs(">=", fouth);
		else if (token_it->kind == TK_INC)		fhputs("++", fouth);
		else if (token_it->kind == TK_DEC)		fhputs("--", fouth);
		else if (token_it->kind == TK_ARROW)		fhputs("->", fouth);
		else if (token_it->kind == TK_MUL_ASS)	fhputs("*=", fouth);
		else if (token_it->kind == TK_DIV_ASS)	fhputs("/=", fouth);
		else if (token_it->kind == TK_MOD_ASS)	fhputs("%%=", fouth);
		else if (token_it->kind == TK_ADD_ASS)	fhputs("+=", fouth);
		else if (token_it->kind == TK_SUB_ASS)	fhputs("-=", fouth);
		else if (token_it->kind == TK_SHL_ASS)	fhputs("<<=", fouth);
		else if (token_it->kind == TK_SHR_ASS)	fhputs(">>=", fouth);
		else if (token_it->kind == TK_SHL)		fhputs("<<", fouth);
		else if (token_it->kind == TK_SHR)		fhputs(">>", fouth);
		else if (token_it->kind == TK_AND)		fhputs("&&", fouth);
		else if (token_it->kind == TK_OR)		fhputs("||", fouth);
		else fhputs("????", fouth);

		token_it = token_it->next(token_it, TRUE);
	}

	fprintf(fout, "\n\nDone\n");	
	fclose(fout);
}


typedef struct type_s *type_p;

#define OPER_POST_INC    2000
#define OPER_POST_DEC    2001
#define OPER_PRE_INC     2002
#define OPER_PRE_DEC     2002
#define OPER_PLUS        2003
#define OPER_MIN         2004

typedef struct expr_s *expr_p;
struct expr_s
{
	int kind;
	int int_val;
	char *str_val;
	int nr_children;
	expr_p children[0];
	type_p type;
};

expr_p new_expr(int kind, int nr_children)
{
	expr_p expr = (expr_p)malloc(sizeof(struct expr_s) + nr_children * sizeof(expr_p));
	expr->kind = kind;
	expr->int_val = 0;
	expr->str_val = NULL;
	expr->nr_children = nr_children;
	for (int i = 0; i < nr_children; i++)
		expr->children[i] = NULL;
	expr->type = NULL;
	return expr;
}

expr_p new_expr_int_value(int value)
{
	expr_p expr = new_expr('0', 0);
	expr->int_val = value;
	return expr;
}

int expr_eval(expr_p expr)
{
	switch (expr->kind)
	{
		case '0': return expr->int_val;
		case '+': return expr_eval(expr->children[0]) + expr_eval(expr->children[1]);
		case '-': return expr_eval(expr->children[0]) - expr_eval(expr->children[1]);
		case '/': return expr_eval(expr->children[0]) / expr_eval(expr->children[1]);
	}
	if (expr->kind < 128)
		printf("Error: expr_eval '%c'\n", expr->kind);
	else
		printf("Error: expr_eval %d\n", expr->kind);
	exit(0);
	return 0;
}

// Types

typedef enum
{
	BT_VOID = 0,
	BT_S8 =  1 | 2 | 4,
	BT_U8 =  1 | 4,
	BT_S16 = 1 | 2 | 8,
	BT_U16 = 1 | 8,
	BT_S32 = 1 | 2 | 12,
	BT_U32 = 1 | 12,
	BT_S64 = 1 | 2 | 16,
	BT_U64 = 1 | 16,
	BT_F   = 32,
	BT_DF  = 33,
	BT_JMP_BUF  = 34,
	BT_FILE     = 35,
} base_type_e;

typedef enum
{
	TYPE_KIND_BASE,
	TYPE_KIND_STRUCT,
	TYPE_KIND_UNION,
	TYPE_KIND_POINTER,
	TYPE_KIND_ARRAY,
	TYPE_KIND_FUNCTION,
} type_kind_e;

typedef struct decl_s *decl_p;

struct type_s
{
	type_kind_e kind;
	base_type_e base_type;
	int size;
	int nr_members;  // TYPE_KIND_STRUCT or TYPE_KIND_UNION
	type_p *members;
	int nr_decls;    // TYPE_KIND_FUNCTION
	decl_p *decls;
	int nr_elems;    // TYPE_KIND_ARRAY
};

type_p new_type(type_kind_e kind, int size, int nr_members)
{
	type_p type = (type_p)malloc(sizeof(struct type_s));
	type->kind = kind;
	type->base_type = 0;
	type->size = size;
	type->nr_members = nr_members;
	if (nr_members > 0)
	{
		type->members = (type_p*)malloc(nr_members * sizeof(type_p));
		for (int i = 0; i < nr_members; i++)
			type->members[i] = NULL;
	}
	else
		type->members = NULL;
	type->nr_decls = 0;
	type->decls = NULL;
	type->nr_elems = 0;
	return type;
}

type_p new_base_type(base_type_e base)
{
	int size = 4;
	switch (base & ~3)
	{
		case  4: size = 1; break;
		case  8: size = 2; break;
		case 12: size = 4; break;
		case 16: size = 8; break;
		case 32: size = 4; break;
		case 33: size = 8; break;
	}
	type_p type = new_type(TYPE_KIND_BASE, size, 0);
	type->base_type = base;
	return type;
}

// Declarations

typedef enum {
	DK_IDENT,
	DK_STRUCT,
	DK_UNION,
	DK_ENUM,
} decl_kind_e;

struct decl_s
{
	decl_kind_e kind;
	char *name;
	type_p type;
	bool is_typedef;
	decl_p prev;
};

decl_p cur_decls = NULL;

void new_decl(decl_kind_e kind, const char *name, type_p type)
{
	decl_p decl = (decl_p)malloc(sizeof(struct decl_s));
	decl->kind = kind;
	decl->name = copystr(name);
	decl->type = type;
	decl->is_typedef = FALSE;
	decl->prev = cur_decls;
	cur_decls = decl;
}

decl_p find_decl(decl_kind_e kind, char *name)
{
	for (decl_p decls = cur_decls; decls != NULL; decls = decls->prev)
		if (strcmp(decls->name, name) == 0)
			return decls;
	return NULL;
}

decl_p find_or_add_decl(decl_kind_e kind, char *name)
{
	decl_p decl = find_decl(kind, name);
	if (decl != NULL)
		return decl;
	new_decl(kind, name, NULL);
	return cur_decls;
}

// Parse functions

void next_token(void)
{
	token_it = token_it->next(token_it, FALSE);
}

bool accept_term(int kind)
{
	if (token_it->kind == kind)
	{
		next_token();
		return TRUE;
	}
	return FALSE;
}

#define FAIL_FALSE { printf("Fail in %s at %d\n", __func__, __LINE__); return FALSE; }
#define FAIL_NULL  { printf("Fail in %s at %d\n", __func__, __LINE__); return NULL; }

expr_p parse_expr(void);
type_p parse_type_specifier(void);

expr_p parse_primary_expr(void)
{
	expr_p expr = NULL;
	if (token_it->kind == 'i')
	{
		expr = new_expr('i', 0);
		expr->str_val = copystr(token_it->token);
		token_it = token_it->next(token_it, FALSE);
	}
	else if (token_it->kind == '0')
	{
		expr = new_expr_int_value(token_it->int_value);
		token_it = token_it->next(token_it, FALSE);
	}
	else if (token_it->kind == '\'')
	{
		expr = new_expr_int_value(token_it->token[0]);
		token_it = token_it->next(token_it, FALSE);
	}
	else if (token_it->kind == '"')
	{
		static char strbuf[6000];
		int len = token_it->int_value;
		memcpy(strbuf, token_it->token, len);
		token_it = token_it->next(token_it, FALSE);
		while (token_it->kind == '"')
		{
			memcpy(strbuf + len, token_it->token, token_it->int_value);
			len += token_it->int_value;
			token_it = token_it->next(token_it, FALSE);
		}
		strbuf[len++] = '\0';
		expr = new_expr('"', 0);
		expr->str_val = (char*)malloc(len);
		memcpy(expr->str_val, strbuf, len);
	}
	else if (accept_term('('))
	{
		type_p type = parse_type_specifier();
		if (type != NULL)
		{
			printf("Cast expr\n");
			if (!accept_term(')'))
				FAIL_NULL
			expr_p subj_expr = parse_primary_expr();
			if (subj_expr == NULL)
				FAIL_NULL;
			expr = new_expr('c', 1);
			expr->children[0] = subj_expr;
			expr->type = type;
		}
		else
		{
			expr = parse_expr(); 
			if (expr == NULL)
				FAIL_NULL
			if (!accept_term(')'))
				FAIL_NULL
		}
	}
	return expr;
}

expr_p parse_assignment_expr(void);

expr_p parse_postfix_expr(void)
{
	expr_p expr = parse_primary_expr();
	if (expr == NULL)
		FAIL_NULL
	for (;;)
	{
		if (accept_term('['))
		{
			expr_p index_expr = parse_expr();
			if (index_expr == NULL)
				FAIL_NULL
			if (!accept_term(']'))
				FAIL_NULL
			expr_p arr_expr = new_expr('[', 2);
			arr_expr->children[0] = expr;
			arr_expr->children[1] = index_expr;
			expr = arr_expr;
		}
		else if (accept_term('('))
		{
			int nr_children = 1;
			expr_p children[20];
			children[0] = expr;
			do
			{
				expr_p child = parse_assignment_expr();
				if (child == NULL)
					FAIL_NULL
				children[nr_children++] = child;
			} while (accept_term(','));
			if (!accept_term(')'))
				FAIL_NULL
			expr = new_expr('(', 1 + nr_children);
			for (int i = 0; i < nr_children; i++)
				expr->children[i] = children[i];
		}
		else if (accept_term('.'))
		{
			if (token_it->kind != 'i')
				FAIL_NULL
			expr_p field_expr = new_expr('.', 1);
			field_expr->str_val = copystr(token_it->token);
			field_expr->children[0] = expr;
			expr = field_expr;
			next_token();
		}
		else if (accept_term(TK_ARROW))
		{
			if (token_it->kind != 'i')
				FAIL_NULL
			expr_p field_expr = new_expr(TK_ARROW, 1);
			field_expr->str_val = copystr(token_it->token);
			field_expr->children[0] = expr;
			expr = field_expr;
			next_token();
		}
		else if (accept_term(TK_INC))
		{
			expr_p post_oper_expr = new_expr(OPER_POST_INC, 1);
			post_oper_expr->children[0] = expr;
			expr = post_oper_expr;
		}
		else if (accept_term(TK_DEC))
		{
			expr_p post_oper_expr = new_expr(OPER_POST_DEC, 1);
			post_oper_expr->children[0] = expr;
			expr = post_oper_expr;
		}
		else
			break;
	}
	return expr;
}

expr_p parse_sizeof_type(void);

expr_p parse_unary_expr(void)
{
	if (accept_term(TK_INC))
	{
		expr_p expr = parse_unary_expr();
		if (expr == NULL)
			FAIL_NULL
		expr_p pre_oper_expr = new_expr(OPER_PRE_INC, 1);
		pre_oper_expr->children[0] = expr;
		return pre_oper_expr;
	}
	if (accept_term(TK_DEC))
	{
		expr_p expr = parse_unary_expr();
		if (expr == NULL)
			FAIL_NULL
		expr_p pre_oper_expr = new_expr(OPER_PRE_DEC, 1);
		pre_oper_expr->children[0] = expr;
		return pre_oper_expr;
	}
	if (accept_term('&'))
	{
		expr_p expr = parse_unary_expr();
		if (expr == NULL)
			FAIL_NULL
		expr_p pre_oper_expr = new_expr('&', 1);
		pre_oper_expr->children[0] = expr;
		return pre_oper_expr;
	}
	if (accept_term('+'))
	{
		expr_p expr = parse_unary_expr();
		if (expr == NULL)
			FAIL_NULL
		expr_p pre_oper_expr = new_expr(OPER_PLUS, 1);
		pre_oper_expr->children[0] = expr;
		return pre_oper_expr;
	}
	if (accept_term('-'))
	{
		expr_p expr = parse_unary_expr();
		if (expr == NULL)
			FAIL_NULL
		expr_p pre_oper_expr = new_expr(OPER_MIN, 1);
		pre_oper_expr->children[0] = expr;
		return pre_oper_expr;
	}
	if (accept_term('~'))
	{
		expr_p expr = parse_unary_expr();
		if (expr == NULL)
			FAIL_NULL
		expr_p pre_oper_expr = new_expr('~', 1);
		pre_oper_expr->children[0] = expr;
		return pre_oper_expr;
	}
	if (accept_term('!'))
	{
		expr_p expr = parse_unary_expr();
		if (expr == NULL)
			FAIL_NULL
		expr_p pre_oper_expr = new_expr('!', 1);
		pre_oper_expr->children[0] = expr;
		return pre_oper_expr;
	}
	if (accept_term(TK_SIZEOF))
	{
		if (accept_term('('))
		{
			expr_p type = parse_sizeof_type();
			if (type == NULL)
				FAIL_NULL
			if (!accept_term(')'))
				FAIL_NULL
		}
		else
		{
			if (parse_unary_expr() == NULL)
				FAIL_NULL
		}
		expr_p expr = new_expr(TK_SIZEOF, 0);
		return expr;		
	}
	
	return parse_postfix_expr();
}

expr_p parse_sizeof_type(void)
{
	expr_p expr = NULL;
	if (accept_term(TK_INT))
		expr = new_expr_int_value(4);
	else if (accept_term(TK_UNSIGNED))
	{
		if (accept_term(TK_INT))
			expr = new_expr_int_value(4);
	}
	else if (accept_term(TK_DOUBLE))
		expr = new_expr_int_value(8);
	else if (accept_term(TK_VOID))
	{
		if (accept_term('*'))
			expr = new_expr_int_value(4);
	}
	else if (accept_term(TK_STRUCT))
	{
		if (token_it->kind == 'i')
		{
			decl_p decl = find_decl(DK_STRUCT, token_it->token);
			if (decl != NULL && decl->type != NULL)
			{
				expr = new_expr_int_value(decl->type->size);
				next_token();
			}
		}
	}
	else if (token_it->kind == 'i')
	{
		decl_p decl = find_decl(DK_IDENT, token_it->token);
		if (decl != NULL && decl->type != NULL)
		{
			expr = new_expr_int_value(decl->type->size);
			next_token();
		}
	}
	if (expr == NULL)
		FAIL_NULL
	while (accept_term('*'))
		expr->int_val = 4;
	return expr;
}

/*

	NT_DEF("cast_expr")
		RULE CHAR_WS('(') NT("abstract_declaration") CHAR_WS(')') NT("cast_expr") TREE("cast")
		RULE NTP("unary_expr")
*/

expr_p parse_expr1(void)
{
	expr_p expr = parse_unary_expr();
	for (;;)
	{
		if (accept_term('*'))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_unary_expr();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr('*', 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term('/'))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_unary_expr();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr('/', 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term('%'))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_unary_expr();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr('%', 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else
			break;
	}
	
	return expr;
}

expr_p parse_expr2(void)
{
	expr_p expr = parse_expr1();
	for (;;)
	{
		if (accept_term('+'))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr1();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr('+', 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term('-'))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr1();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr('-', 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else
			break;
	}
	
	return expr;
}

expr_p parse_expr3(void)
{
	expr_p expr = parse_expr2();
	for (;;)
	{
		if (accept_term(TK_SHL))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr2();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(TK_SHL, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term(TK_SHR))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr2();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(TK_SHR, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else
			break;
	}
	
	return expr;
}

expr_p parse_expr4(void)
{
	expr_p expr = parse_expr3();
	for (;;)
	{
		if (accept_term(TK_EQ))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr3();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(TK_EQ, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term(TK_NE))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr3();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(TK_NE, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term(TK_LE))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr3();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(TK_LE, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term(TK_GE))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr3();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(TK_GE, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term('<'))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr3();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr('<', 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term('>'))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr3();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr('>', 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else
			break;
	}
	
	return expr;
}

expr_p parse_expr5(void)
{
	expr_p expr = parse_expr4();
	for (;;)
	{
		if (accept_term('^'))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr4();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr('^', 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else
			break;
	}
	
	return expr;
}

expr_p parse_expr6(void)
{
	expr_p expr = parse_expr5();
	for (;;)
	{
		if (accept_term('&'))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr5();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr('&', 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else
			break;
	}
	
	return expr;
}

expr_p parse_expr7(void)
{
	expr_p expr = parse_expr6();
	for (;;)
	{
		if (accept_term('|'))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr6();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr('|', 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else
			break;
	}
	
	return expr;
}

expr_p parse_expr8(void)
{
	expr_p expr = parse_expr7();
	for (;;)
	{
		if (accept_term(TK_AND))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr7();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(TK_AND, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else
			break;
	}
	
	return expr;
}

expr_p parse_expr9(void)
{
	expr_p expr = parse_expr8();
	for (;;)
	{
		if (accept_term(TK_OR))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_expr8();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(TK_OR, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else
			break;
	}
	
	return expr;
}

expr_p parse_conditional_expr(void)
{
	expr_p expr = parse_expr9();
	if (accept_term('?'))
	{
		expr_p cond_expr = expr;
		expr_p then_expr = parse_expr9();
		if (then_expr == NULL)
			FAIL_NULL
		if (!accept_term(':'))
			FAIL_NULL
		expr_p else_expr = parse_conditional_expr();
		if (else_expr == NULL)
			FAIL_NULL
		expr = new_expr(TK_OR, 3);
		expr->children[0] = cond_expr;
		expr->children[1] = then_expr;
		expr->children[2] = else_expr;
	}
	
	return expr;
}

expr_p parse_assignment_expr(void)
{
	expr_p expr = parse_conditional_expr();
	for (;;)
	{
		if (accept_term('='))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_assignment_expr();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr('=', 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term(TK_MUL_ASS))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_assignment_expr();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(TK_MUL_ASS, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term(TK_DIV_ASS))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_assignment_expr();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(TK_DIV_ASS, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term(TK_MOD_ASS))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_assignment_expr();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(TK_MOD_ASS, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term(TK_ADD_ASS))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_assignment_expr();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(TK_ADD_ASS, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term(TK_SUB_ASS))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_assignment_expr();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(TK_SUB_ASS, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term(TK_SHL_ASS))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_assignment_expr();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(TK_SHL_ASS, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else if (accept_term(TK_SHR_ASS))
		{
			expr_p lhs = expr;
			expr_p rhs = parse_assignment_expr();
			if (rhs == NULL)
				FAIL_NULL
			expr = new_expr(TK_SHR_ASS, 2);
			expr->children[0] = lhs;
			expr->children[1] = rhs;
		}
		else
			break;
	}
	
	return expr;
}


expr_p parse_expr(void)
{
	expr_p expr = parse_assignment_expr();
	if (expr == NULL)
		FAIL_NULL
	if (accept_term(','))
	{
		int nr_children = 1;
		expr_p children[20];
		children[0] = expr;
		do
		{
			expr_p child = parse_assignment_expr();
			if (child == NULL)
				FAIL_NULL
			children[nr_children++] = child;
		} while (accept_term(','));
		expr = new_expr(',', nr_children);
		for (int i = 0; i < nr_children; i++)
			expr->children[i] = children[i];
	}
	return expr;
}

/*
	NT_DEF("constant_expr")
		RULE NTP("conditional_expr")
*/

bool parse_statements(void)
{
	for (;;)
	{
		if (accept_term(TK_RETURN))
		{
			if (parse_expr() == NULL)
				FAIL_FALSE;
			if (!accept_term(';'))
				FAIL_FALSE;
		}
		else
		{
			expr_p expr = parse_expr();
			(void)expr;
			if (!accept_term(';'))
				break;
		}
	}
	return TRUE;
}

bool parse_declaration(bool is_param)
{
	bool is_typedef = FALSE;
	for (;;)
	{
		if (accept_term(TK_TYPEDEF))
		{
			is_typedef = TRUE;
		}
		else if (accept_term(TK_EXTERN))
		{
		}
		else if (accept_term(TK_INLINE))
		{
		}
		else if (accept_term(TK_STATIC))
		{
		}
		else
			break;
	}
	type_p type = parse_type_specifier();
	printf("type %d %p\n", is_typedef, type);
	if (type == NULL)
		FAIL_FALSE
	do
	{
		while (accept_term('*'))
		{
			type_p pointer_type = new_type(TYPE_KIND_POINTER, 4, 1);
			pointer_type->members[0] = type;
			type = pointer_type;
		}
		decl_p decl = NULL;
		bool as_pointer = FALSE;
		if (token_it->kind == 'i')
		{
			decl = find_or_add_decl(DK_IDENT, token_it->token);
			next_token();
		}
		else if (accept_term('('))
		{
			if (accept_term('*'))
			{
				if (token_it->kind == 'i')
				{
					as_pointer = TRUE;
					decl = find_or_add_decl(DK_IDENT, token_it->token);
					next_token();
				}
			}

			if (!accept_term(')'))
				FAIL_FALSE
		}
		if (decl != NULL)
		{
			for (;;)
			{
				if (accept_term('('))
				{
					decl_p save_decls = cur_decls;
					type_p members[20];
					members[0] = type;
					int nr_members = 1;
					do
					{
						if (accept_term(TK_DASHES))
						{
							members[nr_members++] = NULL;
							break;
						}
						if (!parse_declaration(TRUE))
							FAIL_FALSE;
						members[nr_members++] = cur_decls->type;
					} while (accept_term(','));
					if (!accept_term(')'))
						FAIL_FALSE;
					if (decl->type != NULL)
					{
						decl->type = new_type(TYPE_KIND_FUNCTION, 4, nr_members);
						for (int i = 0; i < nr_members; i++)
							decl->type->members[i] = members[i];
					}
					if (accept_term('{'))
					{
						if (!parse_statements())
							FAIL_FALSE
						return accept_term('}');
					}
					cur_decls = save_decls;
					break;
				}
				else if (accept_term('['))
				{
					if (accept_term(']'))
					{
						type_p ptr_type = new_type(TYPE_KIND_POINTER, 4, 1);
						ptr_type->members[0] = type;
						type = ptr_type;
					}
					else
					{
						expr_p expr = parse_expr();
						if (expr == NULL)
							return FALSE;
						int nr_elems = expr_eval(expr);
						type_p arr_type = new_type(TYPE_KIND_ARRAY, nr_elems * type->size, 1);
						arr_type->members[0] = type;
						arr_type->nr_elems = nr_elems;
						type = arr_type;
						if (!accept_term(']'))
							FAIL_FALSE
					}
				}
				else
					break;
			}
			if (as_pointer)
			{
				type_p ptr_type = new_type(TYPE_KIND_POINTER, 4, 1);
				ptr_type->members[0] = type;
				type = ptr_type;
			}
			decl->type = type;
			decl->is_typedef = is_typedef;
			if (accept_term('='))
			{
				parse_expr();
				// TODO: should be initializer
			}
		}
	} while (!is_param && accept_term(','));
/*
	NT_DEF("declaration")
		RULE
		{ GROUPING
			RULE NTP("storage_class_specifier")
		} SEQL OPTN ADD_CHILD
		NT("type_specifier")
		{ GROUPING
			RULE NT("func_declarator") CHAR_WS('(')
			{ GROUPING
				RULE NTP("parameter_declaration_list") OPTN
				//RULE KEYWORD("void") TREE("voidx")
			} ADD_CHILD
			CHAR_WS(')')
			{ GROUPING
				RULE CHAR_WS(';') TREE("forward")
				RULE CHAR_WS('{') NTP("decl_or_stat") CHAR_WS('}')
			} ADD_CHILD TREE("new_style") WS
			RULE
			{ GROUPING
				RULE NT("declarator")
				{ GROUPING
					RULE WS CHAR_WS('=') NTP("initializer")
				} OPTN ADD_CHILD TREE("decl_init")
			} SEQL OPTN ADD_CHILD { CHAIN CHAR_WS(',') } CHAR_WS(';') TREE("decl")
		}
*/
	return is_param || accept_term(';');
}

type_p parse_struct_or_union_specifier(decl_kind_e decl_kind);
type_p parse_enum_specifier(void);

type_p parse_type_specifier(void)
{
	if (accept_term(TK_CONST))
	{
		/*
		if (accept_term(TK_CHAR))
		{
			return new_base_type(BT_S8);
		}
		if (accept_term(TK_INT))
		{
			return new_base_type(BT_S16);
		}
		if (accept_term(TK_UNSIGNED))
		{
			if (accept_term(TK_CHAR))
			{
				return new_base_type(BT_U8);
			}
		}
		if (accept_term(TK_VOID))
		{
			return new_base_type(BT_VOID);
		}
		FAIL_NULL
		*/
	}
	if (accept_term(TK_CHAR))
	{
		if (accept_term(TK_CONST))
		{
		}
		return new_base_type(BT_S8);
	}
	if (accept_term(TK_UNSIGNED))
	{
		if (accept_term(TK_CHAR))
		{
			return new_base_type(BT_U8);
		}
		if (accept_term(TK_SHORT))
		{
			return new_base_type(BT_U16);
		}
		if (accept_term(TK_LONG))
		{
			if (accept_term(TK_LONG))
			{
				return new_base_type(BT_U64);
			}
			return new_base_type(BT_U32);
		}
		if (accept_term(TK_INT))
		{
			return new_base_type(BT_U32);
		}
		return new_base_type(BT_S32);
	}
	if (accept_term(TK_SHORT))
	{
		return new_base_type(BT_U16);
	}
	if (accept_term(TK_INT))
	{
		return new_base_type(BT_S32);
	}
	if (accept_term(TK_LONG))
	{
		if (accept_term(TK_DOUBLE))
		{
			return new_base_type(BT_DF);
		}
		if (accept_term(TK_LONG))
		{
			return new_base_type(BT_S64);
		}
		return new_base_type(BT_S32); 
	}
	if (accept_term(TK_FLOAT))
	{
		return new_base_type(BT_F);
	}
	if (accept_term(TK_DOUBLE))
	{
		return new_base_type(BT_DF);
	}
	if (accept_term(TK_VOID))
	{
		return new_base_type(BT_VOID);
	}
	if (accept_term(TK_STRUCT))
	{
		return parse_struct_or_union_specifier(DK_STRUCT);
	}
	if (accept_term(TK_UNION))
	{
		return parse_struct_or_union_specifier(DK_UNION);
	}
	if (accept_term(TK_ENUM))
	{
		return parse_enum_specifier();
	}
	if (token_it->kind == 'i')
	{
		decl_p decl = find_decl(DK_IDENT, token_it->token);
		if (decl == NULL)
		{
			printf("Ident %s has no declaration\n", token_it->token);
			FAIL_NULL
		}
		else if (decl->type == NULL)
			FAIL_NULL
		else if (decl->is_typedef)
		{
			printf("Ident %s is typedef\n", decl->name);
			next_token();
			return decl->type;
		}
		else
		{
			printf("Ident %s is not a typedef\n", token_it->token);
			FAIL_NULL
		}
	}
	FAIL_NULL
}

type_p parse_struct_or_union_specifier(decl_kind_e decl_kind)
{
	type_kind_e type_kind = decl_kind == DK_STRUCT ? TYPE_KIND_STRUCT : TYPE_KIND_UNION;
	type_p type = NULL;
	if (token_it->kind == 'i')
	{
		decl_p decl = find_or_add_decl(decl_kind, token_it->token);
		if (decl->type == NULL)
		{
			decl->type = new_type(type_kind, 0, 0);
		}
		type = decl->type;
		next_token();
	}
	if (accept_term('{'))
	{
		decl_p save_decls = cur_decls;
		int nr_decls = 0;
		decl_p decls[200];
		int size = 0;
		do
		{
			if (!parse_declaration(FALSE))
				FAIL_NULL
			decls[nr_decls++] = cur_decls;
			int decl_size = cur_decls->type != 0 ? cur_decls->type->size : 0;
			if (decl_kind == DK_STRUCT)
				size += decl_size;
			else if (decl_size > size)
				size = decl_size;
		} while (!accept_term('}'));
		if (type == NULL)
		{
			type = new_type(type_kind, size, 0);
		}
		else
			type->size = size;
		if (nr_decls >= type->nr_decls)
		{
			type->nr_decls = nr_decls;
			type->decls = (decl_p*)malloc(nr_decls * sizeof(decl_p));
		}
		for (int i = 0; i < nr_decls; i++)
			type->decls[i] = decls[i];
		cur_decls = save_decls;
	}
	return type;
}

/*
	NT_DEF("type_specifier")
		RULE KEYWORD("const") KEYWORD("char") TREE("const_char")
		RULE KEYWORD("const") KEYWORD("int") TREE("const_int")
		RULE KEYWORD("const") KEYWORD("unsigned") KEYWORD("char") TREE("const_u_char")
		RULE KEYWORD("const") KEYWORD("void") TREE("const_void")
		RULE KEYWORD("const") IDENT TREE("const")
		RULE KEYWORD("const") NTP("struct_or_union_specifier")
		RULE KEYWORD("char") KEYWORD("const") TREE("const_char")
		RULE KEYWORD("char") TREE("char")
		RULE KEYWORD("unsigned") KEYWORD("char") TREE("u_char")
		RULE KEYWORD("unsigned") KEYWORD("short") TREE("u_short")
		RULE KEYWORD("unsigned") KEYWORD("long") KEYWORD("long") TREE("u_long_long_int")
		RULE KEYWORD("unsigned") KEYWORD("long") TREE("u_long_int")
		RULE KEYWORD("unsigned") KEYWORD("int") TREE("u_int")
		RULE KEYWORD("unsigned") TREE("u_int")
		RULE KEYWORD("short") TREE("short")
		RULE KEYWORD("int") TREE("int")
		RULE KEYWORD("long") KEYWORD("double") TREE("long_double")
		RULE KEYWORD("long") KEYWORD("long") TREE("long_long_int")
		RULE KEYWORD("long") TREE("long")
		//RULE KEYWORD("float") TREE("float")
		RULE KEYWORD("double") TREE("double")
		RULE KEYWORD("void") TREE("void")
		RULE NTP("struct_or_union_specifier")
		RULE NTP("enum_specifier")
		RULE IDENT PASS

	NT_DEF("struct_or_union_specifier")
		RULE KEYWORD("struct") IDENT_OPT
		{ GROUPING
			RULE CHAR_WS('{')
			{ GROUPING
				RULE NTP("struct_declaration_or_anon")
			} SEQL ADD_CHILD
			CHAR_WS('}') PASS
		} OPTN ADD_CHILD TREE("struct")
		RULE KEYWORD("union") IDENT_OPT
		{ GROUPING
			RULE CHAR_WS('{')
			{ GROUPING
				RULE NTP("struct_declaration_or_anon")
			} SEQL ADD_CHILD
			CHAR_WS('}') PASS
		} OPTN ADD_CHILD TREE("union")

	NT_DEF("struct_declaration_or_anon")
		RULE NTP("struct_or_union_specifier") CHAR_WS(';')
		RULE NTP("struct_declaration")

	NT_DEF("struct_declaration")
		RULE NT("type_specifier") NT("declarator") SEQL { CHAIN CHAR_WS(',') } CHAR_WS(';') TREE("struct_declaration")
		//RULE NT("type_specifier") NT("struct_declaration") TREE("type")
		//RULE NT("struct_declarator") SEQL { CHAIN CHAR_WS(',') } CHAR_WS(';') TREE("strdec")

	//NT_DEF("struct_declarator")
	//	RULE NT("declarator")
	//	{ GROUPING
	//		RULE CHAR_WS(':') NT("constant_expr")
	//	} OPTN ADD_CHILD TREE("record_field")
*/

type_p parse_enum_specifier()
{
	type_p type = NULL;
	if (token_it->kind == 'i')
	{
		decl_p decl = find_or_add_decl(DK_ENUM, token_it->token);
		if (decl->type == NULL)
		{
			decl->type = new_type(DK_ENUM, 4, 0);
		}
		type = decl->type;
		next_token();
	}
	int next_enum_val = 0;
	if (accept_term('{'))
	{
		for (;;)
		{
			if (token_it->kind != 'i')
				FAIL_NULL
			decl_p const_decl = find_or_add_decl(DK_IDENT, token_it->token);
			next_token();
			if (accept_term('='))
			{
				expr_p expr = parse_conditional_expr();
				if (expr == NULL)
					FAIL_NULL
				next_enum_val = expr_eval(expr);
			}
			// TODO
			(void)const_decl;
			next_enum_val++;
			if (!accept_term(','))
				break;
			if (token_it->kind == '}')
				break;
		}
		if (!accept_term('}'))
			FAIL_NULL
	}
	if (type == NULL)
		type = new_type(DK_ENUM, 4, 0);
	return type;
}

/*
	NT_DEF("enum_specifier")
		RULE KEYWORD("enum") IDENT_OPT CHAR_WS('{') NT("enumerator") SEQL { CHAIN CHAR_WS(',') } CHAR_WS('}') TREE("enum")

	NT_DEF("enumerator")
		RULE IDENT
		{ GROUPING
			RULE CHAR_WS('=') NTP("constant_expr")
		} OPTN ADD_CHILD TREE("enumerator")

	NT_DEF("func_declarator")
		RULE CHAR_WS('*')
		{ GROUPING
			RULE KEYWORD("const") TREE("const")
		} OPTN ADD_CHILD NT("func_declarator") TREE("pointdecl")
		RULE CHAR_WS('(') NT("func_declarator") CHAR_WS(')')
		RULE IDENT PASS

	NT_DEF("declarator")
		RULE CHAR_WS('*')
		{ GROUPING
			RULE KEYWORD("const") TREE("const")
		} OPTN ADD_CHILD NT("declarator") TREE("pointdecl")
		RULE CHAR_WS('(') NT("declarator") CHAR_WS(')') TREE("brackets")
		RULE WS IDENT PASS
		REC_RULEC CHAR_WS('[') NT("constant_expr") OPTN CHAR_WS(']') TREE("array")
		REC_RULEC CHAR_WS('(') NT("abstract_declaration_list") OPTN CHAR_WS(')') TREE("function")

	NT_DEF("abstract_declaration_list")
		RULE
			NT("abstract_declaration") SEQL BACK_TRACKING { CHAIN CHAR_WS(',') }
			{ GROUPING
				RULE CHAR_WS(',') CHAR('.') CHAR('.') CHAR_WS('.') TREE("varargs")
			} OPTN ADD_CHILD TREE("abstract_declaration_list")

	NT_DEF("parameter_declaration_list")
		RULE
			NT("parameter_declaration") SEQL BACK_TRACKING { CHAIN CHAR_WS(',') }
			{ GROUPING
				RULE CHAR_WS(',') CHAR('.') CHAR('.') CHAR_WS('.') TREE("varargs")
			} OPTN ADD_CHILD TREE("parameter_declaration_list")

	NT_DEF("ident_list")
		RULE IDENT
		{ GROUPING
			RULE CHAR_WS(',')
			{ GROUPING
				RULE CHAR('.') CHAR('.') CHAR_WS('.') TREE("varargs")
				RULE NT("ident_list") TREE("ident_list")
			}
		} OPTN ADD_CHILD TREE("ident_list")

	NT_DEF("parameter_declaration")
		RULE NT("type_specifier")
		{ GROUPING
			RULE NTP("declarator")
			RULE NTP("abstract_declarator")
		} ADD_CHILD TREE("parameter_declaration")

	NT_DEF("abstract_declaration")
		RULE NT("type_specifier")
		{ GROUPING
			RULE NTP("declarator")
			RULE NTP("abstract_declarator")
		} ADD_CHILD TREE("abstract_declarator")
		//RULE NT("type_specifier") NT("parameter_declaration") TREE("type")
		//RULE NTP("abstract_declarator")

	NT_DEF("abstract_declarator")
		RULE CHAR_WS('*')
		{ GROUPING
			RULE KEYWORD("const") TREE("const")
		} OPTN ADD_CHILD NT("abstract_declarator") TREE("abs_pointdecl")
		RULE CHAR_WS('(') NT("abstract_declarator") CHAR_WS(')') TREE("abs_brackets")
		RULE
		REC_RULEC CHAR_WS('[') NT("constant_expr") OPTN CHAR_WS(']') TREE("abs_array")
		REC_RULEC CHAR_WS('(') NT("parameter_declaration_list") CHAR_WS(')') TREE("abs_func")

	NT_DEF("initializer")
		RULE NTP("assignment_expr")
		RULE CHAR_WS('{') NT("initializer") SEQL { CHAIN CHAR_WS(',') } CHAR(',') OPTN WS CHAR_WS('}') TREE("initializer")

	NT_DEF("decl_or_stat")
		RULE NT("declaration") SEQL OPTN NT("statement") SEQL OPTN TREE("decl_or_stat")

	NT_DEF("statement")
		RULE
		{ GROUPING
			RULE
			{ GROUPING
				RULE IDENT TREE("open_label")
				RULE KEYWORD("case") NT("constant_expr") TREE("case")
				RULE KEYWORD("default") TREE("default")
			} ADD_CHILD CHAR_WS(':') NT("statement") TREE("label")
			RULE CHAR_WS('{') NTP("decl_or_stat") CHAR_WS('}')
		}
		RULE
		{ GROUPING
			RULE NTP("expr") OPTN CHAR_WS(';')
			RULE KEYWORD("if") WS CHAR_WS('(') NT("expr") CHAR_WS(')') NT("statement")
			{ GROUPING
				RULE KEYWORD("else") NTP("statement")
			} OPTN ADD_CHILD TREE("if")
			RULE KEYWORD("switch") WS CHAR_WS('(') NT("expr") CHAR_WS(')') NT("statement") TREE("switch")
			RULE KEYWORD("while") WS CHAR_WS('(') NT("expr") CHAR_WS(')') NT("statement") TREE("while")
			RULE KEYWORD("do") NT("statement") KEYWORD("while") WS CHAR_WS('(') NT("expr") CHAR_WS(')') CHAR_WS(';') TREE("do")
			RULE KEYWORD("for") WS CHAR_WS('(') NT("expr") OPTN CHAR_WS(';')
			{ GROUPING
				RULE WS NTP("expr")
			} OPTN ADD_CHILD CHAR_WS(';')
			{ GROUPING
				RULE WS NTP("expr")
			} OPTN ADD_CHILD CHAR_WS(')') NT("statement") TREE("for")
			RULE KEYWORD("goto") IDENT CHAR_WS(';') TREE("goto")
			//RULE KEYWORD("continue") CHAR_WS(';') TREE("cont")
			//RULE KEYWORD("break") CHAR_WS(';') TREE("break")
			RULE KEYWORD("return") NT("expr") OPTN CHAR_WS(';') TREE("ret")
		}

	NT_DEF("root")
		RULE
		WS
		{ GROUPING
			RULE NT("declaration")
		} SEQL OPTN END PASS

*/


void add_base_type(const char *name, base_type_e base)
{
	new_decl(DK_IDENT, name, new_base_type(base));
	cur_decls->is_typedef = TRUE;
}

void add_predefined_types()
{
	add_base_type("uint16_t", BT_U16);
	add_base_type("uint32_t", BT_U32);
	add_base_type("int32_t", BT_S32);
	add_base_type("uint8_t", BT_U8);
	add_base_type("size_t", BT_U32);
	add_base_type("jmp_buf", BT_JMP_BUF);
	add_base_type("FILE", BT_FILE);
}

int main(int argc, char *argv[])
{
	bool only_preprocess = FALSE;
	for (int i = 1; i < argc; i++)
		if (strcmp(argv[i], "-E") == 0)
			only_preprocess = TRUE;

	env_p one_source_env;
	env_p tcc_version_env;
	env_p ldouble_size_env;
	file_iterator_p input_it;
	line_splice_iterator_p splice_it;
	comment_strip_iterator_p comment_it;
	include_iterator_p include_it;
	tokenizer_p tokenizer_it;
	conditional_iterator_p conditional_it;
	expand_iterator_p expand_it;
	
	one_source_env = get_env("ONE_SOURCE", TRUE);
	one_source_env->tokens = new_int_token(1);
	tcc_version_env = get_env("TCC_VERSION", TRUE);
	tcc_version_env->tokens = new_str_token("\"1.0\"");
	ldouble_size_env = get_env("LDOUBLE_SIZE", TRUE);
	ldouble_size_env->tokens = new_int_token(8);
	input_it = new_file_iterator("tcc_sources/tcc.c");
	splice_it = new_line_splice_iterator(&input_it->base);
	comment_it = new_comment_strip_iterator(&splice_it->base);
	include_it = new_include_iterator(&comment_it->base);
	tokenizer_it = new_tokenizer(&include_it->base);
	conditional_it = new_conditional_iterator(include_it, &tokenizer_it->base);
	expand_it = new_expand_iterator(&conditional_it->base);
	
	token_it = (token_iterator_p)expand_it;

	token_it = token_it->next(token_it, TRUE);

	if (only_preprocess)
	{
		output_preprocessor("tcc_p.c");
		return 0;
	}
	add_predefined_types();
	
	while (parse_declaration(FALSE))
	{
	}
	if (token_it != 0)
	{
		printf("Parsed till %s %d.%d: %d '%s'\n", token_it->filename, token_it->line, token_it->column, token_it->kind, token_it->token);
	}	
	
	return 0;
}
