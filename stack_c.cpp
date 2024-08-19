// http://www.unixwiz.net/techtips/x86-jumps.html
#include <stdio.h>
#include <string.h>
#include <cstdint>
#include <malloc.h>

FILE *fin;
FILE *fout;
char cur_char;

void read_char()
{
	if (feof(fin))
		cur_char = '\0';
	else
	{
		cur_char = fgetc(fin);
		if (feof(fin))
			cur_char = '\0';
	}
	//printf("read char %c\n", cur_char);
}

char sym;
char token[1000];
int32_t int_value;

struct Keyword
{
	const char *name;
	char sym;
};
#define NR_KEYWORDS 12
Keyword keywords[NR_KEYWORDS] = {
	{ "func",		'F' },
	{ "const",		'C' },
	{ "var",		'V' },
	{ "loop",		'L' },
	{ "break",		'B' },
	{ "continue",	'D' },
	{ "then",		'T' },
	{ "else" ,		'E' },
	{ "and",		'A' },
	{ "or",			'O' },
	{ "ret",		'R' },
	{ "pop",		'P' },
};

void get_token()
{
	int i = 0;
	for (;;)
	{
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
		if (cur_char == '\0')
		{
			sym = '\0';
			return;
		}

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
			for (int i = 0; i < NR_KEYWORDS; i++)
				if (strcmp(keywords[i].name, token) == 0)
				{
					sym = keywords[i].sym;
					break;
				}
			return;
		}
		else if (cur_char == '\'' || cur_char == '"')
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
				token[i++] = cur_char;
				if (cur_char == '\\')
				{
					read_char();
					token[i++] = cur_char;
				}
			}
			token[i] = '\0';
			return;
		}
		else if (('0' <= cur_char && cur_char <= '9') || cur_char == '-')
		{
			token[i++] = cur_char;
			int sign = 1;
			if (cur_char == '-')
			{
				sign = -1;
				read_char();
			}
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
						while ('0' <= cur_char && cur_char <= '7')
						{
							int_value = 8 * int_value + cur_char - '0';
							read_char();
						}							
					}
				}
				else
				{
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
			else
			{
				sym = 's';
				while (cur_char > ' ')
				{
					token[i++] = cur_char;
					read_char();
				}
				while (cur_char > ' ');
				break;
			}
		}
		else
		{
			sym = 's';
			do
			{
				token[i++] = cur_char;
				read_char();
			}
			while (cur_char > ' ');
			break;
		}
	}
	token[i] = 0;
	if (sym == 's' && i == 1)
		sym = token[0];
}

struct Var
{
	char type;
	char name[50];
	int pos;
	int32_t value;
};

Var vars[500];
int nr_vars = 0;

struct String
{
	char *value;
	String *next;
};
String *strings = 0;

int nr_for_string(const char *s)
{
	printf("# nr_for_string %s\n", s);
	int nr = 0;
	String **ref_string = &strings;
	for (; (*ref_string) != 0; ref_string = &(*ref_string)->next, nr++)
		if (strcmp((*ref_string)->value, s) == 0)
			return nr;
	(*ref_string) = (String*)malloc(sizeof(String));
	(*ref_string)->value = (char*)malloc(strlen(s) + 1);
	strcpy((*ref_string)->value, s);
	(*ref_string)->next = 0;
	return nr;
}

#define MAX_NESTING 100

int nesting[MAX_NESTING];
int nesting_depth = 0;
char nesting_type[MAX_NESTING];
int nesting_id[MAX_NESTING];


void add_function(const char *name)
{
	vars[nr_vars].type = 'F';
	strcpy(vars[nr_vars].name, name);
	nr_vars++;
} 

int main(int argc, char *argv[])
{
	// Add predefined system functions
	add_function("sys_open");
	add_function("sys_close");
	add_function("sys_fgetc");
	add_function("sys_fputc");
	add_function("sys_malloc");

	fout = stdout;

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
			printf("Cannot open file '%s'\n", argv[1]);
			return 0;
		}
	}
	
	int id;

	char function_name[50];

	read_char();
	
	get_token();
	while (true)
	{
		if (sym == '\0')
			break;
		
		if (sym == 'F')
		{
			get_token();
			if (sym != 'i')
			{
				printf("Expecting name after 'func'\n");
				return 0;
			}
			strcpy(function_name, token);
			bool found = false;
			for (int i = 0; i < nr_vars; i++)
				if (strcmp(token, vars[i].name) == 0)
				{
					found = true;
					break;
				}
			if (!found)
				add_function(token);
			get_token();
			if (sym == ';')
			{
			}
			else if (sym == '{')
			{
				//printf("start function %s\n", function_name);
				fprintf(fout, "\n:%s\n\tpop_eax\n\tmov_[ebp],eax\n\tpop_eax\n", function_name);
				nesting_type[nesting_depth] = ' ';
				nesting[nesting_depth++] = nr_vars;
				id = 1;
			}
			else
			{
				printf("Expect ; or { after function name\n");
				return 0;
			}
		}
		else if (sym == 'C')
		{
			get_token();
			if (sym != 'i')
			{
				printf("Expecting name after 'const'\n");
				return 0;
			}
			vars[nr_vars].type = 'C';
			strcpy(vars[nr_vars].name, token);
			get_token();
			if (sym != '0')
			{
				printf("Expecting number after 'const' <name>\n");
				return 0;
			}
			vars[nr_vars].value = int_value;
			nr_vars++;
		}
		else if (sym == 'V')
		{
			get_token();
			if (sym != 'i')
			{
				printf("Expecting name after 'const'\n");
				return 0;
			}
			vars[nr_vars].type = nesting_depth == 0 ? 'G' : 'L';
			strcpy(vars[nr_vars].name, token);
			if (nesting_depth > 0)
			{
				vars[nr_vars].pos = nr_vars - nesting[nesting_depth - 1] + 1;
			} 
			nr_vars++;
		}
		else if (sym == 'L')
		{
			get_token();
			if (sym != '{')
			{
				printf("expecting '{' after 'loop'\n");
			}
			fprintf(fout, ":_%s_loop%d\n", function_name, id);
			nesting_type[nesting_depth] = 'L';
			nesting_id[nesting_depth] = id++;
			nesting[nesting_depth++] = nr_vars;
		}
		else if (sym == 'B' || sym == 'C')
		{
			int i = nesting_depth - 1;
			for (; i >= 0; i--)
				if (nesting_type[i] == 'L')
					break;
			if (i < 0)
			{
				printf("'break' outside 'loop'\n");
				return 0;
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
				printf("expecting '{' after 'then'\n");
			}
			fprintf(fout, "\ttest_eax,eax           # then\n\tpop_eax\n\tje %%_%s_else%d\n", function_name, id);
			nesting_type[nesting_depth] = 'T';
			nesting_id[nesting_depth] = id++;
			nesting[nesting_depth++] = nr_vars;
		}
		else if (sym == 'E')
		{
			printf("unexpected 'else'\n");
			return 0;
		}
		else if (sym == 'A')
		{
			get_token();
			if (sym != '{')
			{
				printf("expecting '{' after 'and'\n");
			}
			fprintf(fout, "\ttest_eax              # and\n\tjne %%_%s_and_end%d\tpop_eax\n\n", function_name, id);
			nesting_type[nesting_depth] = 'A';
			nesting_id[nesting_depth] = id++;
			nesting[nesting_depth++] = nr_vars;
		}
		else if (sym == 'O')
		{
			get_token();
			if (sym != '{')
			{
				printf("expecting '{' after 'or'\n");
			}
			fprintf(fout, "\ttest_eax              # or\n\tje %%_%s_or_end%d\tpop_eax\n\n", function_name, id);
			nesting_type[nesting_depth] = 'O';
			nesting_id[nesting_depth] = id++;
			nesting[nesting_depth++] = nr_vars;
		}
		else if (sym == '{')
		{
			nesting_type[nesting_depth] = ' ';
			nesting[nesting_depth++] = nr_vars;
		}
		else if (sym == '}')
		{
			if (nesting_depth == 0)
			{
				printf("To many }\n");
				return 0;
			}
			nr_vars = nesting[--nesting_depth];
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
						printf("expecting '{' after 'else'\n");
						return 0;
					}
					fprintf(fout, "\tjmp %%_%s_else_end%d\n", function_name, nesting_id[nesting_depth]);
					fprintf(fout, ":_%s_else%d\n", function_name, nesting_id[nesting_depth]);
					nesting_type[nesting_depth] = 'E';
					nesting[nesting_depth++] = nr_vars;
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
			fprintf(fout, "\tmov_ebx,[ebp]         # ret\n\tpush_ebx\n\tret\n");
		}	
		else if (sym == 'P')
		{
			fprintf(fout, "\tpop_eax               # pop\n");
		}	
		else if (sym == 'i')
		{
			bool found = false;
			for (int i = nr_vars - 1; i >= 0; i--)
				if (strcmp(token, vars[i].name) == 0)
				{
					found = true;
					//fprintf(fout, "# Ident %s type %c", token, vars[i].type);
					//if (vars[i].type == 'L')
					//	fprintf(fout, "\tpos %d", vars[i].pos);
					//else if (vars[i].type == 'C')
					//	fprintf(fout, "\tvalue %d", vars[i].value);
					//fprintf(fout, "\n");
					if (vars[i].type == 'G')
						fprintf(fout, "\tpush_eax              # global\n\tmov_eax, &%s\n", token);
					else if (vars[i].type == 'F')
						fprintf(fout, "\tpush_eax              # function\n\tmov_eax, &%s\n", token);
					else if (vars[i].type == 'C')
						fprintf(fout, "\tpush_eax              # const\n\tmov_eax, %%%d\n", vars[i].value);
					else if (vars[i].type == 'L')
					{
						fprintf(fout, "\tpush_eax              # local %s\n\tlea_eax,[ebp+DWORD] %%%d\n", token, 4 * vars[i].pos);
					}
					break;
				}
			if (!found)
			{
				fprintf(fout, "## Ident %s is not defined\n", token);

			}
		}
		else if (sym == '0')
		{
			fprintf(fout, "\tpush_eax              # push %d\n\tmov_eax, %%%d\n", int_value, int_value);
		}
		else if (sym == '"')
		{
			int nr = nr_for_string(token);
			fprintf(fout, "\tpush_eax              # push string %s\n\tmov_eax, &string_%d\n", token, nr);
		}
		else if (sym == '?')
		{
			fprintf(fout, "\tmov_eax,[eax]         # ?\n");
		}
		else if (sym == '=')
		{
			fprintf(fout, "\tpop_ebx               # =\n\tmov_[eax],ebx\n\tpop_eax\n");
		}
		else if (sym == '+')
		{
			fprintf(fout, "\tpop_ebx               # +\n\tadd_eax,ebx\n");
		}
		else if (sym == '-')
		{
			fprintf(fout, "\tpop_ebx               # -\n\tsub_ebx,eax\n\tmov_eax,ebx\n");
		}
		else if (sym == 's')
		{
			if (token[0] == ':')
			{
				fprintf(fout, "%s_%s\n", token, function_name);
			}
			else if (strcmp(token, "?1") == 0)
			{
				fprintf(fout, "\tmov_al,[eax]          # ?1\n\tmovzx_eax,al\n");
			}
			else if (strcmp(token, "=1") == 0)
			{
				fprintf(fout, "\tpop_ebx               # =1\n\tmov_[eax],bl\n\tpop_eax\n");
			}
			else if (strcmp(token, "()") == 0)
			{
				int nr = nr_vars - nesting[0] + 1;
				//printf(" call at %d offset %d\n", nr_vars, nr);
				fprintf(fout, "\tadd_ebp, %%%d         # ()\n\tcall_eax\n\tsub_ebp, %%%d\n", 4 * nr, 4 * nr);
			}
			else if (strcmp(token, "==") == 0)
			{
				fprintf(fout, "\tpop_ebx               # ==\n\tcmp_eax_ebx\n\tsete_al\n\tmovzx_eax,al\n");
			}
			else if (strcmp(token, "!=") == 0)
			{
				fprintf(fout, "\tpop_ebx               # !=\n\tcmp_eax_ebx\n\tsetne_al\n\tmovzx_eax,al\n");
			}
			else if (strcmp(token, "<") == 0)
			{
				fprintf(fout, "\tpop_ebx               # <\n\tcmp_eax_ebx\n\tsetb_al\n\tmovzx_eax,al\n");
			}
			else if (strcmp(token, "<=") == 0)
			{
				fprintf(fout, "\tpop_ebx               # <=\n\tcmp_eax_ebx\n\tsetbe_al\n\tmovzx_eax,al\n");
			}
			else if (strcmp(token, ">") == 0)
			{
				fprintf(fout, "\tpop_ebx               # >\n\tcmp_eax_ebx\n\tseta_al\n\tmovzx_eax,al\n");
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
			else
			{
				fprintf(fout, "# %c |%s|\n", sym, token);
			}	
		}
		else
		{
			fprintf(fout, "# %c |%s|\n", sym, token);
		}
		
		get_token();
	}
	
	fprintf(fout, "\n:ELF_data\n\n");
	fprintf(fout, ":SYS_MALLOC NULL\n");
	int nr = 0;
	for (String *string = strings; string != 0; string = string->next, nr++)
		fprintf(fout, ":string_%d  \"%s\"\n", nr, string->value);
	fprintf(fout, "\n:ELF_end\n");	

	return 0;
}