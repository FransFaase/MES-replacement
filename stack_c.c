/*	Stack_C: Compiler for Stack C language to i386 Assembly

	This is to adapted to parse output produced by the tcc_cc.c
	C compiler. For that reason, it only uses keywords that are
	also used in C, where the following keywords are used in a
	slightly different way:
	- void: A function definition
	- int: A variable definition (default 32 bits)


	Assembly generation
	- ebp register contains pointer to local variable stack
	- first location of variable stack used for return address function
Notes:
- http://www.unixwiz.net/techtips/x86-jumps.html
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>

// Constants

#define MAX_TOKEN_LENGTH 1000
#define MAX_NR_VARIABLES 500
#define MAX_VARIABLE_LENGTH 50
#define MAX_NESTING 100

// Boolean definition

#define TRUE 1
#define FALSE 0
typedef int bool;

// Global variables

FILE *fin;
FILE *fout;
FILE *ferr;
char cur_char = '\0';
int cur_line = 1;

// Read charachter

void read_char(void)
{
	if (feof(fin))
		cur_char = '\0';
	else
	{
		if (cur_char == '\n')
			cur_line++;
		cur_char = fgetc(fin);
		if (feof(fin))
			cur_char = '\0';
	}
}

char sym;
char token[MAX_TOKEN_LENGTH+1];
int int_value;

typedef struct
{
	const char *name;
	char sym;
} Keyword;
#define NR_KEYWORDS 10
Keyword keywords[NR_KEYWORDS] = {
	{ "void",		'F' },
	{ "const",		'C' },
	{ "int",		'V' },
	{ "do",			'L' },
	{ "break",		'B' },
	{ "continue",	'D' },
	{ "if",			'T' },
	{ "else" ,		'E' },
	{ "return",		'R' },
	{ "goto",		'G' },
};

int error = 0;

void get_token(void)
{
	int i = 0;
	// Skip white spaces, but echo comments starting with # till end of the line
	while ((cur_char != '\0' && cur_char <= ' ') || cur_char == '#')
	{
		if (cur_char == '#')
		{
			do
			{
				fputc(cur_char, fout);
				read_char();
			}
			while (cur_char != '\0' && cur_char != '\n');
			fputc('\n', fout);
		}
		else
			read_char();
	}

	// Check for end of file
	if (cur_char == '\0')
	{
		sym = '\0';
		return;
	}

	// Check for identifier
	if (('a' <= cur_char && cur_char <= 'z') || ('A' <= cur_char && cur_char <= 'Z') || cur_char == '_')
	{
		sym = 'i';
		do
		{
			token[i++] = cur_char;
			read_char();
		}
		while (('a' <= cur_char && cur_char <= 'z') || ('A' <= cur_char && cur_char <= 'Z') || cur_char == '_' || ('0' <= cur_char && cur_char <= '9'));
		token[i] = '\0';

		if (i > MAX_VARIABLE_LENGTH)
		{
			fprintf(ferr, "ERROR %d: Variable '%s' is longer than %d characters\n", cur_line, token, MAX_VARIABLE_LENGTH);
			error = 1;
			sym = '\0';
			return;
		}

		// Recognize keywords
		for (int i = 0; i < NR_KEYWORDS; i++)
			if (strcmp(keywords[i].name, token) == 0)
			{
				sym = keywords[i].sym;
				break;
			}
		return;
	}

	// Check for character or string
	if (cur_char == '\'' || cur_char == '"')
	{
		sym = cur_char;
		char quote = cur_char;
		for (;;)
		{
			read_char();
			if (cur_char == '\0')
				break;
			if (cur_char == quote)
			{
				read_char();
				break;
			}
			if (i > MAX_TOKEN_LENGTH)
			{
				fprintf(ferr, "ERROR %d: String longer than %d characters\n", cur_line, MAX_TOKEN_LENGTH);
				error = 1;
				sym = '\0';
				return;
			}
			if (cur_char == '\\')
			{
				read_char();
				if (cur_char == '0')
					cur_char = '\0';
				else if (cur_char == 'n')
					cur_char = '\n';
				else if (cur_char == 'r')
					cur_char = '\r';
				else if (cur_char == 't')
					cur_char = '\t';
				token[i++] = cur_char;
			}
			else
				token[i++] = cur_char;
		}
		token[i] = '\0';
		return;
	}

	// Check for number or '->' symbol
	if (('0' <= cur_char && cur_char <= '9') || cur_char == '-')
	{
		token[i++] = cur_char;
		int sign = 1;
		if (cur_char == '-')
		{
			sign = -1;
			read_char();
			// check of '->' symbol
			if (cur_char == '>')
			{
				sym = 's';
				token[i++] = cur_char;
				token[i] = '\0';
				read_char();
				return;
			}
			if (cur_char <= ' ')
			{
				sym = '-';
				token[i] = '\0';
				return;
			}
		}
		// Check for number
		if ('0' <= cur_char && cur_char <= '9')
		{
			sym = '0';
			int_value = 0;
			if (cur_char == '0')
			{
				read_char();
				if (cur_char == 'x')
				{
					read_char();
					// Parse hexadecimal number
					while (1)
					{
						if ('0' <= cur_char && cur_char <= '9')
							int_value = 16 * int_value + cur_char - '0';
						else if ('a' <= cur_char && cur_char <= 'f')
							int_value = 16 * int_value + cur_char - 'a' + 10;
						else if ('A' <= cur_char && cur_char <= 'F')
							int_value = 16 * int_value + cur_char - 'A' + 10;
						else
							break;
						read_char();
					}
				}
				else
				{
					// Parse octal number
					while ('0' <= cur_char && cur_char <= '7')
					{
						int_value = 8 * int_value + cur_char - '0';
						read_char();
					}
				}
			}
			else
			{
				// Parse decimal number
				while ('0' <= cur_char && cur_char <= '9')
				{
					int_value = 10 * int_value + cur_char - '0';
					read_char();
				}
			}
			int_value = sign * int_value;
			sprintf(token, "%d", int_value);
			return; 
		}
	}

	// Parse symbol till next white space
	sym = 's';
	do
	{
		token[i++] = cur_char;
		read_char();
	}
	while (cur_char > ' ');

	token[i] = '\0';
	if (i == 1)
		sym = token[0];
}

// Identifiers

typedef struct
{
	char type;   // 'F': Function, 'C': constant, 'G': global variable, 'L': local variable
	char name[MAX_VARIABLE_LENGTH+1];
	int pos;     // position for local variable
	int size;    // size for variable
	int value;   // value for constant
} ident_t;

ident_t idents[MAX_NR_VARIABLES];
int nr_idents = 0;
int pos = 0;

// String constants

typedef struct string_s *string_p;
struct string_s
{
	char *value;
	string_p next;
};
string_p strings = 0;

void save_print_string(FILE *fout, const char *s)
{
	for (; *s != '\0'; s++)
		if (*s == '\n')
			fprintf(fout, "\\n");
		else if (*s == '\t')
			fprintf(fout, "\\t");
		else if (*s == '\\' || *s == '"')
			fprintf(fout, "\\%c", *s);
		else if (*s < ' ')
			fprintf(fout, "?");
		else
			fprintf(fout, "%c", *s);
}

int nr_for_string(const char *s)
{
	//printf("# nr_for_string %s\n", s);
	int nr = 0;
	string_p *ref_string = &strings;
	for (; (*ref_string) != 0; ref_string = &(*ref_string)->next, nr++)
		if (strcmp((*ref_string)->value, s) == 0)
			return nr;
	(*ref_string) = (string_p)malloc(sizeof(struct string_s));
	(*ref_string)->value = (char*)malloc(strlen(s) + 1);
	strcpy((*ref_string)->value, s);
	(*ref_string)->next = 0;
	return nr;
}

int nesting_depth = 0;
int nesting_nr_vars[MAX_NESTING];
int nesting_pos[MAX_NESTING];
char nesting_type[MAX_NESTING];
int nesting_id[MAX_NESTING];


void add_function(const char *name)
{
	idents[nr_idents].type = 'F';
	strcpy(idents[nr_idents].name, name);
	nr_idents++;
} 

int main(int argc, char *argv[])
{
	ferr = stderr;

	// Add predefined system functions
	add_function("sys_int80");
	add_function("sys_malloc");

	fout = stdout;

	// Copy contents of stack_c_intro.M1
	fin = fopen("stack_c_intro.M1", "r");
	if (fin != 0)
	{
		for (;;)
		{
			cur_char = fgetc(fin);
			if (feof(fin))
				break;
			fputc(cur_char, fout);
		}
		fclose(fin);
	}
	
	fin = stdin;
	
	if (argc == 2)
	{
		fin = fopen(argv[1], "r");
		if (fin == 0)
		{
			fprintf(ferr, "ERROR %d: Cannot open file '%s'\n", cur_line, argv[1]);
			return 1;
		}
	}
	
	int id;

	char function_name[MAX_VARIABLE_LENGTH+1];

	read_char();
	
	get_token();
	while (TRUE)
	{
		if (sym == '\0')
			break;
		
		if (sym == 'F')
		{
			// Function definition
			get_token();
			if (sym != 'i')
			{
				fprintf(ferr, "ERROR %d: Expecting name after 'void' for function\n", cur_line);
				return 1;
			}
			// Save the function name
			strcpy(function_name, token);
			// Add it, if is has not been found
			{
				bool found = FALSE;
				for (int i = 0; i < nr_idents; i++)
					if (strcmp(token, idents[i].name) == 0)
					{
						found = TRUE;
						break;
					}
				if (!found)
				{
					if (nr_idents >= MAX_NR_VARIABLES)
					{
						fprintf(ferr, "ERROR %d: More than %d variables\n", cur_line, MAX_NR_VARIABLES);
						return 1;
					}
					add_function(token);
				}
			}
			get_token();
			if (sym == ';')
			{
				// Forward definition
			}
			else
			{
				pos = 1;
				if (sym != '{')
				{
					fprintf(ferr, "ERROR %d: Expect ; or { after function name\n", cur_line);
					return 1;
				}
				//printf("start function %s\n", function_name);
				// Copy return address to first location in variable stack
				fprintf(fout, "\n:%s\n\tpop_eax\n\tmov_[ebp],eax\n\tpop_eax\n", function_name);
				nesting_type[nesting_depth] = ' ';
				nesting_nr_vars[nesting_depth] = nr_idents;
				nesting_pos[nesting_depth] = pos;
				nesting_depth++;
				id = 1;
			}
		}
		else if (sym == 'C')
		{
			// Constant definition
			get_token();
			if (sym != 'i')
			{
				fprintf(ferr, "ERROR %d: Expecting name after 'const'\n", cur_line);
				return 1;
			}
			if (nr_idents >= MAX_NR_VARIABLES)
			{
				fprintf(ferr, "ERROR %d: More than %d variables\n", cur_line, MAX_NR_VARIABLES);
				return 1;
			}
			idents[nr_idents].type = 'C';
			strcpy(idents[nr_idents].name, token);
			get_token();
			if (sym != '0')
			{
				fprintf(ferr, "ERROR %d: Expecting number after 'const' <name>\n", cur_line);
				return 1;
			}
			idents[nr_idents].value = int_value;
			nr_idents++;
		}
		else if (sym == 'V')
		{
			// Variable definition
			get_token();
			int size = 1;
			if (sym == '0')
			{
				size = int_value;
				get_token();
			}
			if (sym != 'i')
			{
				fprintf(ferr, "ERROR %d: Expecting name after 'int'\n", cur_line);
				return 1;
			}
			if (nr_idents >= MAX_NR_VARIABLES)
			{
				fprintf(ferr, "ERROR %d: More than %d variables\n", cur_line, MAX_NR_VARIABLES);
				return 1;
			}
			idents[nr_idents].type = nesting_depth == 0 ? 'G' : 'L';
			strcpy(idents[nr_idents].name, token);
			idents[nr_idents].size = size;
			if (nesting_depth > 0)
			{
				idents[nr_idents].pos = pos;
				pos += size;
			}
			nr_idents++;
		}
		else if (sym == 'L')
		{
			get_token();
			if (sym != '{')
			{
				fprintf(ferr, "ERROR %d: expecting '{' after 'do'\n", cur_line);
				return 1;
			}
			fprintf(fout, ":_%s_loop%d\n", function_name, id);
			if (nesting_depth >= MAX_NESTING)
			{
				fprintf(ferr, "ERROR %d: Nesting deeper than %d\n", cur_line, MAX_NESTING);
				return 1;
			}
			nesting_type[nesting_depth] = 'L';
			nesting_id[nesting_depth] = id++;
			nesting_nr_vars[nesting_depth] = nr_idents;
			nesting_pos[nesting_depth] = pos;
			nesting_depth++;
		}
		else if (sym == 'B' || sym == 'D')
		{
			int i = nesting_depth - 1;
			for (; i >= 0; i--)
				if (nesting_type[i] == 'L')
					break;
			if (i < 0)
			{
				fprintf(ferr, "ERROR %d: 'break' outside 'loop'\n", cur_line);
				return 1;
			}
			if (sym == 'B')
				fprintf(fout, "\tjmp %%_%s_loop_end%d\n", function_name, nesting_id[i]);
			else
				fprintf(fout, "\tjmp %%_%s_loop%d\n", function_name, nesting_id[i]);
		}
		else if (sym == 'T')
		{
			get_token();
			if (sym != '{')
			{
				fprintf(ferr, "ERROR %d: expecting '{' after 'if'\n", cur_line);
				return 1;
			}
			fprintf(fout, "\ttest_eax,eax          # if\n\tpop_eax\n\tje %%_%s_else%d\n", function_name, id);
			if (nesting_depth >= MAX_NESTING)
			{
				fprintf(ferr, "ERROR %d: Nesting deeper than %d\n", cur_line, MAX_NESTING);
				return 1;
			}
			nesting_type[nesting_depth] = 'T';
			nesting_id[nesting_depth] = id++;
			nesting_nr_vars[nesting_depth] = nr_idents;
			nesting_pos[nesting_depth] = pos;
			nesting_depth++;
		}
		else if (sym == 'E')
		{
			fprintf(ferr, "ERROR %d: unexpected 'else'\n", cur_line);
			return 1;
		}
		else if (sym == '{')
		{
			nesting_type[nesting_depth] = ' ';
			nesting_nr_vars[nesting_depth] = nr_idents;
			nesting_pos[nesting_depth] = pos;
			nesting_depth++;
		}
		else if (sym == '}')
		{
			if (nesting_depth == 0)
			{
				fprintf(ferr, "ERROR %d: To many }\n", cur_line);
				return 1;
			}
			nesting_depth--;
			nr_idents = nesting_nr_vars[nesting_depth];
			pos = nesting_pos[nesting_depth];
			if (nesting_type[nesting_depth] == 'L')
				fprintf(fout, "\tjmp %%_%s_loop%d\n:_%s_loop_end%d\n", function_name, nesting_id[nesting_depth], function_name, nesting_id[nesting_depth]);
			else if (nesting_type[nesting_depth] == 'T')
			{
				get_token();
				if (sym == 'E')
				{
					get_token();
					if (sym != '{')
					{
						fprintf(ferr, "ERROR %d: expecting '{' after 'else'\n", cur_line);
						return 1;
					}
					fprintf(fout, "\tjmp %%_%s_else_end%d\n", function_name, nesting_id[nesting_depth]);
					fprintf(fout, ":_%s_else%d\n", function_name, nesting_id[nesting_depth]);
					if (nesting_depth >= MAX_NESTING)
					{
						fprintf(ferr, "ERROR %d: Nesting deeper than %d\n", cur_line, MAX_NESTING);
						return 1;
					}
					nesting_type[nesting_depth] = 'E';
					nesting_nr_vars[nesting_depth] = nr_idents;
					nesting_pos[nesting_depth] = pos;
					nesting_depth++;
				}
				else
				{
					fprintf(fout, ":_%s_else%d # no else\n", function_name, nesting_id[nesting_depth]);
					continue; // to skip the call to get_token at the end of the while loop
				}				
			}
			else if (nesting_type[nesting_depth] == 'E')
				fprintf(fout, ":_%s_else_end%d\n", function_name, nesting_id[nesting_depth]);
			else if (nesting_type[nesting_depth] == 'A')
				fprintf(fout, ":_%s_and_end%d\n", function_name, nesting_id[nesting_depth]);
			else if (nesting_type[nesting_depth] == 'O')
				fprintf(fout, ":_%s_or_end%d\n", function_name, nesting_id[nesting_depth]);
		}
		else if (sym == 'R')
		{
			fprintf(fout, "\tmov_ebx,[ebp]         # return\n\tpush_ebx\n\tret\n");
		}
		else if (sym == ';')
		{
			fprintf(fout, "\tpop_eax               # ;\n");
		}
		else if (sym == '$')
		{
			fprintf(fout, "\tpush_eax              # $ (dup)\n");
		}
		else if (sym == 'G')
		{
			get_token();
			if (sym != 'i')
			{
				fprintf(ferr, "Expecting label after 'goto'\n");
				return 0;
			}
			fprintf(fout, "\tjmp %%%s_%s\n", token, function_name);
		}
		else if (sym == 'i')
		{
			int i = nr_idents - 1;
			for (; i >= 0; i--)
				if (strcmp(token, idents[i].name) == 0)
					break;
			if (i >= 0)
			{
				//fprintf(fout, "# Ident %s type %c", token, idents[i].type);
				//if (idents[i].type == 'L')
				//	fprintf(fout, "\tpos %d", idents[i].pos);
				//else if (idents[i].type == 'C')
				//	fprintf(fout, "\tvalue %d", idents[i].value);
				//fprintf(fout, "\n");
				if (idents[i].type == 'G')
					fprintf(fout, "\tpush_eax              # %s (global)\n\tmov_eax, &%s\n", token, token);
				else if (idents[i].type == 'F')
					fprintf(fout, "\tpush_eax              # %s (function)\n\tmov_eax, &%s\n", token, token);
				else if (idents[i].type == 'C')
					fprintf(fout, "\tpush_eax              # %d (const %s)\n\tmov_eax, %%%d\n", idents[i].value, token, idents[i].value);
				else if (idents[i].type == 'L')
					fprintf(fout, "\tpush_eax              # %s (local)\n\tlea_eax,[ebp+DWORD] %%%d\n", token, 4 * idents[i].pos);
			}
			else
			{
				fprintf(ferr, "ERROR %d: Ident %s is not defined\n", cur_line, token);
				error = 1;
			}
		}
		else if (sym == '0')
		{
			fprintf(fout, "\tpush_eax              # %d\n\tmov_eax, %%%d\n", int_value, int_value);
		}
		else if (sym == '"')
		{
			int nr = nr_for_string(token);
			fprintf(fout, "\tpush_eax              # '");
			save_print_string(fout, token);
			fprintf(fout, "'\n\tmov_eax, &string_%d\n", nr);
		}
		else if (sym == '\'')
		{
			if (' ' < token[0] && token[0] < 127)
				fprintf(fout, "\tpush_eax              # '%c'\n\tmov_eax, %%%d\n", token[0], token[0]);
			else
				fprintf(fout, "\tpush_eax              # %d\n\tmov_eax, %%%d\n", token[0], token[0]);
		}
		else if (sym == '?')
		{
			fprintf(fout, "\tmov_eax,[eax]         # ?\n");
		}
		else if (sym == '=')
		{
			fprintf(fout, "\tpop_ebx               # =\n\tmov_[ebx],eax\n");
		}
		else if (sym == '+')
		{
			fprintf(fout, "\tpop_ebx               # +\n\tadd_eax,ebx\n");
		}
		else if (sym == '-')
		{
			fprintf(fout, "\tpop_ebx               # -\n\tsub_ebx,eax\n\tmov_eax,ebx\n");
		}
		else if (sym == '*')
		{
			fprintf(fout, "\tpop_ebx               # *\n\timul_eax,ebx\n");
		}
		else if (sym == '/')
		{
			fprintf(fout, "\tmov_ebx,eax           # /\n\tpop_eax\n\tcdq\n\tidiv_ebx\n");
		}
		else if (sym == '%')
		{
			fprintf(fout, "\tmov_ebx,eax           # %%\n\tpop_eax\n\tcdq\n\tidiv_ebx\n\tmov_eax,edx");
		}
		else if (sym == '<')
		{
			fprintf(fout, "\tpop_ebx               # <\n\tcmp_eax_ebx\n\tsetb_al\n\tmovzx_eax,al\n");
		}
		else if (sym == '>')
		{
			fprintf(fout, "\tpop_ebx               # >\n\tcmp_eax_ebx\n\tseta_al\n\tmovzx_eax,al\n");
		}
		else if (sym == '!')
		{
			fprintf(fout, "\ttest_eax,eax          # !\n\tsete_al\n\tmovzx_eax,al\n");
		}
		else if (sym == 's')
		{
			if (token[0] == ':')
			{
				fprintf(fout, "%s_%s\n", token, function_name);
			}
			else if (strcmp(token, "=:") == 0)
			{
				fprintf(fout, "\tpop_ebx               # =:\n\tmov_[eax],ebx\n\tpop_eax\n");
			}
			else if (strcmp(token, "?1") == 0)
			{
				fprintf(fout, "\tmov_al,[eax]          # ?1\n\tmovzx_eax,al\n");
			}
			else if (strcmp(token, "=1") == 0)
			{
				fprintf(fout, "\tpop_ebx               # =1\n\tmov_[ebx],al\n");
			}
			else if (strcmp(token, "()") == 0)
			{
				//int nr = pos - nesting_nr_vars[0] + 1;
				//printf(" call at %d offset %d\n", pos, nr);
				fprintf(fout, "\tadd_ebp, %%%d         # ()\n\tcall_eax\n\tsub_ebp, %%%d\n", 4 * pos, 4 * pos);
			}
			else if (strcmp(token, "==") == 0)
			{
				fprintf(fout, "\tpop_ebx               # ==\n\tcmp_eax_ebx\n\tsete_al\n\tmovzx_eax,al\n");
			}
			else if (strcmp(token, "!=") == 0)
			{
				fprintf(fout, "\tpop_ebx               # !=\n\tcmp_eax_ebx\n\tsetne_al\n\tmovzx_eax,al\n");
			}
			else if (strcmp(token, "<=") == 0)
			{
				fprintf(fout, "\tpop_ebx               # <=\n\tcmp_eax_ebx\n\tsetbe_al\n\tmovzx_eax,al\n");
			}
			else if (strcmp(token, ">=") == 0)
			{
				fprintf(fout, "\tpop_ebx               # >=\n\tcmp_eax_ebx\n\tsetae_al\n\tmovzx_eax,al\n");
			}
			else if (strcmp(token, "<s") == 0)
			{
				fprintf(fout, "\tpop_ebx               # <s\n\tcmp_eax_ebx\n\tsetl_al\n\tmovzx_eax,al\n");
			}
			else if (strcmp(token, "<=s") == 0)
			{
				fprintf(fout, "\tpop_ebx               # <=s\n\tcmp_eax_ebx\n\tsetle_al\n\tmovzx_eax,al\n");
			}
			else if (strcmp(token, ">s") == 0)
			{
				fprintf(fout, "\tpop_ebx               # >s\n\tcmp_eax_ebx\n\tsetg_al\n\tmovzx_eax,al\n");
			}
			else if (strcmp(token, ">=s") == 0)
			{
				fprintf(fout, "\tpop_ebx               # >=sfv\n\tcmp_eax_ebx\n\tsetge_al\n\tmovzx_eax,al\n");
			}
			else if (strcmp(token, "<<") == 0)
			{
				fprintf(fout, "\tmov_ecx,eax           # <<\n\tpop_eax\n\tshl_eax,cl\n");
			}
			else if (strcmp(token, ">>") == 0)
			{
				fprintf(fout, "\tmov_ecx,eax           # <<\n\tpop_eax\n\tshr_eax,cl\n");
			}
			else if (strcmp(token, "&&") == 0)
			{
				get_token();
				if (sym != '{')
				{
					fprintf(ferr, "ERROR %d: expecting '{' after '&&'\n", cur_line);
					return 1;
				}
				fprintf(fout, "\ttest_eax,eax          # &&\n\tje %%_%s_and_end%d\n\tpop_eax\n", function_name, id);
				if (nesting_depth >= MAX_NESTING)
				{
					fprintf(ferr, "ERROR %d: Nesting deeper than %d\n", cur_line, MAX_NESTING);
					return 1;
				}
				nesting_type[nesting_depth] = 'A';
				nesting_id[nesting_depth] = id++;
				nesting_nr_vars[nesting_depth] = nr_idents;
				nesting_pos[nesting_depth] = pos;
				nesting_depth++;
			}
			else if (strcmp(token, "||") == 0)
			{
				get_token();
				if (sym != '{')
				{
					fprintf(ferr, "ERROR %d: expecting '{' after '||'\n", cur_line);
					return 1;
				}
				fprintf(fout, "\ttest_eax,eax          # ||\n\tjne %%_%s_or_end%d\n\tpop_eax\n", function_name, id);
				if (nesting_depth >= MAX_NESTING)
				{
					fprintf(ferr, "ERROR %d: Nesting deeper than %d\n", cur_line, MAX_NESTING);
					return 1;
				}
				nesting_type[nesting_depth] = 'O';
				nesting_id[nesting_depth] = id++;
				nesting_nr_vars[nesting_depth] = nr_idents;
				nesting_pos[nesting_depth] = pos;
				nesting_depth++;
			}
			else if (strcmp(token, "->") == 0)
			{
				get_token();
				if (sym != 'i')
				{
					fprintf(ferr, "ERROR %d: Expecting const ident after '->'. Found %s\n", cur_line, token);
					return 1;
				}
				int i = nr_idents - 1;
				for (; i >= 0; i--)
					if (strcmp(token, idents[i].name) == 0)
						break;
				if (i >= 0 && idents[i].type == 'C')
				{
					fprintf(fout, "\tmov_eax,[eax]         # ->\n\tadd_eax, %%%d\n", idents[i].value);
				}
				else
				{
					fprintf(ferr, "ERROR %d: Ident %s is not defined\n", cur_line, token);
					error = 1;
				}
			}
			else if (strcmp(token, "><") == 0)
			{
				fprintf(fout, "\tmov_ebx,eax          # >< swap\n\tpop_eax\n\tpush_ebx\n");
			}
			else
			{
				fprintf(ferr, "ERROR %d: Sym %c token |%s| not supported\n", cur_line, sym, token);
				error = 1;
			}
		}
		else
		{
			fprintf(ferr, "ERROR %d: Sym %c token |%s| not supported\n", cur_line, sym, token);
			error = 1;
		}
		
		get_token();
	}
	
	fprintf(fout, "\n:ELF_data\n\n");
	fprintf(fout, ":SYS_MALLOC NULL\n");
	int nr = 0;
	for (string_p string = strings; string != 0; string = string->next, nr++)
	{
		fprintf(fout, ":string_%d  ", nr);
		bool safe_string = TRUE;
		int len = 1;
		for (const char *s = string->value; *s != '\0'; s++, len++)
			if (*s == '"' || (*s < ' ' && *s != '\n' && *s != '\t'))
				safe_string = FALSE;
		if (safe_string)
			fprintf(fout, "\"%s\"", string->value);
		else
		{
			for (const char *s = string->value; *s != '\0'; s++)
				fprintf(fout, "!%u ", *s);
			fprintf(fout, "!0");
		}
		fprintf(fout, "\n");
	}
	for (int i = 0; i < nr_idents; i++)
		if (idents[i].type == 'G')
		{
			fprintf(fout, ":%s", idents[i].name);
			for (int j = 0; j < idents[i].size; j++)
				printf("%sNULL", j % 8 == 0 ? "\n\t" : " ");
			printf("\n");
		}
	fprintf(fout, "\n:ELF_end\n");	

	return error;
}