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

char* copystr(char* str)
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
#define TK_FOR			1009
#define TK_GOTO			1010
#define TK_IF			1011
#define TK_INLINE		1012
#define TK_INT			1013
#define TK_LONG			1014
#define TK_SHORT		1015
#define TK_SIZEOF		1016
#define TK_STATIC		1017
#define TK_STRUCT		1018
#define TK_SWITCH		1019
#define TK_THEN			1020
#define TK_TYPEDEF		1021
#define TK_UNION		1022
#define TK_UNSIGNED		1023
#define TK_VOID			1024
#define TK_WHILE		1025
#define TK_H_ELSE		1026
#define TK_H_ELIF		1027
#define TK_H_ENDIF		1028
#define TK_H_DEFINE		1029
#define TK_DEFINED		1030
#define TK_H_IF			1031
#define TK_H_IFDEF		1032
#define TK_H_IFNDEF		1033
#define TK_H_INCLUDE	1034
#define TK_H_UNDEF		1035
#define TK_H_ERROR		1036

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
	bool _at_start_of_line;
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
		if (ch == '\n')
		{
			tokenizer->_at_start_of_line = TRUE;
		}
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
	token_it->filename = it->filename;
	token_it->line = it->line;
	token_it->column = it->column;
	if (ch == '\n')
	{
		token_it->kind = '\n';
	}
	else if (tokenizer->_at_start_of_line && ch == '#')
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
		else if (eqstr("for",      token_it->token)) token_it->kind = TK_FOR;
		else if (eqstr("goto",     token_it->token)) token_it->kind = TK_GOTO;
		else if (eqstr("if",       token_it->token)) token_it->kind = TK_IF;
		else if (eqstr("inline",   token_it->token)) token_it->kind = TK_INLINE;
		else if (eqstr("int",      token_it->token)) token_it->kind = TK_INT;
		else if (eqstr("long",     token_it->token)) token_it->kind = TK_LONG;
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
				token_it->kind = 's';
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
		else
		{
			it_next(it);
		}
	}
	done: token_it->token[i] = 0;
	printf("tokenizer_next %d '%s'\n", token_it->kind, token_it->token);
	tokenizer->_at_start_of_line = FALSE;
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
	tokenizer->_at_start_of_line = TRUE;
	tokenizer->_char_iterator = char_iterator;
	tokenizer->base.token = _malloc(6000);
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
	while (it->_token_it->kind == TK_AND)
	{
		token_it_next(it->_token_it, FALSE);
		value = value || conditional_iterator_parse_and_expr(it);
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
				//printf("INCLUDE '%s'\n", include_path);
				input_it = new_file_iterator(include_path);
				if (input_it->base.ch != 0)
				{
					splice_it = new_line_splice_iterator(&input_it->base);
					comment_it = new_comment_strip_iterator(&splice_it->base);
					include_iterator_add(it->_source_it, &comment_it->base);
					token_it_next(it->_token_it, FALSE);
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

typedef struct expand_macro_iterator_s* expand_macro_iterator_p;
struct expand_macro_iterator_s
{
	token_iterator_t base;
	tokens_p param_tokens;
	tokens_p tokens;
	token_iterator_p _rest_it;
	env_p _macro;
	tokens_p args[10];
};

token_iterator_p expand_macro_iterator_next(token_iterator_p token_it, bool dummy)
{
	expand_macro_iterator_p it = (expand_macro_iterator_p)token_it;
	if (it->param_tokens != 0)
	{
		token_it->kind = it->param_tokens->kind;
		token_it->token = it->param_tokens->token;
		token_it->int_value = it->param_tokens->int_value;
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
		token_it->kind = token->kind;
		token_it->token = token->token;
		token_it->int_value = token->int_value;
		token_it->filename = token->filename;
		token_it->line = token->line;
		token_it->column = token->column;
		
		return token_it;
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



int main()
{
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
	token_iterator_p token_it;
	int i;
	char ch;
	
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
	
	FILE *fout = fopen("tcc_p.c", "w");
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
		else if (token_it->kind == '"')
		{
			fhputs("\"", fouth);
			for (i = 0; i < token_it->int_value ; i = i + 1)
			{
				ch = token_it->token[i];
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
			fhputs("\"", fouth);
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

	return 0;
}
