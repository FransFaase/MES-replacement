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

void *_malloc(size_t size)
{
	//printf("malloc %ld ", size);
	fflush(stdout);
	void* r = malloc(size);
	//printf("%p\n", r);
	return r;
}



typedef void (*FUNCTION)();

#include "tcc_cc.c"

struct string_iterator_s
{
	char_iterator_t base;
	const char *_s;
};
typedef struct string_iterator_s *string_iterator_p;

void string_iterator_next(string_iterator_p it)
{
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
	it->base.ch = *it->_s;
	if (it->base.ch != 0)
		it->_s++;
	//printf("string_iterarot_next '%c'\n", it->base.ch);
}


string_iterator_p new_string_iterator(const char *s)
{
	string_iterator_p it = malloc(sizeof(struct string_iterator_s));
	it->_s = s;
	it->base.ch = '\n';
	it->base.line = 0;
	it->base.column = 0;
	it->base.filename = "<string>";
	it->base.next = string_iterator_next;
	string_iterator_next(it);
	return it;
}

void test_line_splice_iterator(const char *in, const char *out)
{
	string_iterator_p str_it = new_string_iterator(in);
	line_splice_iterator_p it = new_line_splice_iterator(&str_it->base);
	
	char result[101];
	int i = 0;
	while (it->base.ch != 0)
	{
		if (i < 100)
			result[i++] = it->base.ch;
		it->base.next(it);
	}
	result[i] = 0;
	if (strcmp(result, out) != 0)
		printf("test_line_splice\nexpect: '%s'\nGot: '%s'\n\n", out, result);
}

void test_commit_strip_iterator(const char *in, const char *out)
{
	string_iterator_p str_it = new_string_iterator(in);
	comment_strip_iterator_p it = new_comment_strip_iterator(&str_it->base);
	
	char result[101];
	int i = 0;
	while (it->base.ch != 0)
	{
		//printf("i = %d\n", i);
		if (i < 100)
			result[i++] = it->base.ch;
		it->base.next(it);
	}
	result[i] = 0;
	if (strcmp(result, out) != 0)
		printf("test_commit_strip_iterator\nexpect: '%s'\nGot: '%s'\n\n", out, result);
}

/*
int main(int argc, char *argv[])
{
	test_line_splice_iterator("","");
	test_line_splice_iterator("ab","ab");
	test_line_splice_iterator("a  b","a b");
	test_line_splice_iterator("a\t b","a b");
	test_line_splice_iterator("a \tb","a b");
	test_line_splice_iterator("ab \t","ab ");
	test_line_splice_iterator("a\nb","a\nb");
	test_line_splice_iterator("a\\\nb","ab");
	test_line_splice_iterator("a \\\nb","a b");
	test_line_splice_iterator("a\t\\\nb","a b");
	
	test_commit_strip_iterator("","");
	test_commit_strip_iterator("ab","ab");*/
//	test_commit_strip_iterator("a/* */b","a b");
//	test_commit_strip_iterator("ab/* */","ab ");
//	test_commit_strip_iterator("a/*\n /*/b","a b");
/*	test_commit_strip_iterator("a// \nb","a\nb");
	test_commit_strip_iterator("ab// \n","ab\n");
	
	init_keywords();
	
	env_p one_source_env = get_env("ONE_SOURCE", TRUE);
	one_source_env->tokens = new_int_token(1);
	env_p tcc_version_env = get_env("TCC_VERSION", TRUE);
	tcc_version_env->tokens = new_str_token("\"1.0\"");
	
	file_iterator_p input_it = new_file_iterator("tcc_sources/tcc.c");
	//string_iterator_p input_it = new_string_iterator("\"xx\"  \"yy\")");
	line_splice_iterator_p splice_it = new_line_splice_iterator(&input_it->base);
	comment_strip_iterator_p comment_it = new_comment_strip_iterator(&splice_it->base);
	include_iterator_p include_it = new_include_iterator(&comment_it->base);
	tokenizer_p tokenizer_it = new_tokenizer(&include_it->base);
	expand_iterator_p expand_it = new_expand_iterator(&include_it->base, tokenizer_it);
	
	token_iterator_p token_it = &expand_it->base;
	
	token_it->next(token_it, TRUE);
	char* cur_filename = 0;
	int cur_line = 0;
	while (token_it->kind != 0)
	{
		if (token_it->line != cur_line || token_it->filename != cur_filename)
		{
			printf("\n");
			for (int i = 1; i < token_it->column; i++)
				printf(" ");
			cur_line = token_it->line;
			cur_filename = token_it->filename;
		}
		if (token_it->kind == 'i')
			printf(" %s", token_it->token);
		else if (token_it->kind == '"')
		{
			printf("\"");
			for (int i = 0; i < token_it->int_value ; i++)
			{
				char ch = token_it->token[i];
				if (ch == '\n')
					printf("\\n");
				else if (ch == '\r')
					printf("\\r");
				else if (ch == '\0')
					printf("\\0");
				else if (' ' <= ch && ch < 127)
					printf("%c", ch);
				else
					printf("\\%o", ch);
			}
			printf("\"");
		}
		else if (token_it->kind == '0')
			printf(" %d", token_it->int_value);
		else if (token_it->kind < 127)
			printf(" %c", token_it->kind);
		else if (token_it->kind >= TK_KEYWORD)
			printf(" %s", keywords[token_it->kind - TK_KEYWORD]);
		else
		{
			switch (token_it->kind)
			{
				case TK_D_HASH	: printf(" ##"); break;
				case TK_EQ		: printf(" =="); break;
				case TK_NE		: printf(" !="); break;
				case TK_LE		: printf(" <="); break;
				case TK_GE		: printf(" >="); break;
				case TK_INC		: printf(" ++"); break;
				case TK_DEC		: printf(" --"); break;
				case TK_ARROW	: printf(" ->"); break;
				case TK_MUL_ASS : printf(" *="); break;
				case TK_DIV_ASS	: printf(" /="); break;
				case TK_MOD_ASS	: printf(" %%="); break;
				case TK_ADD_ASS	: printf(" +="); break;
				case TK_SUB_ASS	: printf(" -="); break;
				case TK_SHL_ASS	: printf(" <<="); break;
				case TK_SHR_ASS	: printf(" >>="); break;
				case TK_SHL		: printf(" <<"); break;
				case TK_SHR		: printf(" >>"); break;
				case TK_AND		: printf(" &&"); break;
				case TK_OR		: printf(" ||"); break;
			}
		}
		token_it->next(token_it, TRUE);
	}
	return 0;
}
*/