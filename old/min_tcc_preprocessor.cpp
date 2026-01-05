#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

// ---------------------------------

char *copystr(const char *str)
{
	char *new_str = (char*)malloc(strlen(str) + 1);
	strcpy(new_str, str);
	return new_str;
}

FILE *fout = stdout;

#define MAX_FILENAME_LEN 500

// ---------------------------------

const char *tcc_source_dir = "tcc_sources";

// ---------------------------------

class Env
{
public:
	const char *name;
	const char *value;
	Env *next;
	
	Env(const char *n, const char *v) : name(copystr(n)), value(copystr(v)), next(0) {}
};

Env *environment = 0;

void set_env(const char *name, const char *value)
{
	//fprintf(stdout, "\n/* set_env '%s' = '%s' */", name, value);
	Env **ref_env = &environment;
	for (; *ref_env != 0; ref_env = &(*ref_env)->next)
		if (strcmp((*ref_env)->name, name) == 0)
		{
			(*ref_env)->value = copystr(value);
			return;
		}
	(*ref_env) = new Env(name, value);
}

void del_env(const char *name)
{
	for (Env **ref_env = &environment; *ref_env != 0; ref_env = &(*ref_env)->next)
		if (strcmp((*ref_env)->name, name) == 0)
		{
			(*ref_env) = (*ref_env)->next;
			return;
		}
}

const char *get_env(const char *name)
{
	for (Env *env = environment; env != 0; env = env->next)
		if (strcmp(env->name, name) == 0)
			return env->value;
	return 0;
}

// ---------------------------------

char large_buffer[10001];

class Parser
{
public:
	const char *name;
	int line_nr;
	FILE *fout;
	
	Parser(const char *n, FILE *fo) : name(n), line_nr(0), fout(fo), if_level(-1), skip_level(0)
	{
		char path[MAX_FILENAME_LEN+1];
		snprintf(path, MAX_FILENAME_LEN, "%s/%s", tcc_source_dir, name);
		f = fopen(path, "r");
		if (f == 0)
			fprintf(stderr, "Failed to open '%s'\n", path);
	}
	~Parser()
	{
		if (f != 0) fclose(f);
	}
	
	void parse(bool in_if = false)
	{
		if (f == 0) return;
		
		while (fgets(line, 400, f))
		{
			line_nr++;
			
			s = line;
			while (*s == ' ' || *s == '\t') s++;
			
			if (*s == '#')
			{
				s++;
				while (*s == ' ' || *s == '\t') s++;
				
				parse_token();
				if (skip_level > 0)
				{
					if (is_ident("ifdef") || is_ident("ifndef") || is_ident("if"))
						skip_level++;
					else if (is_ident("endif"))
					{
						skip_level--;
						if (skip_level == 0)
						{
							if_level--;
						}
					}
					else if (is_ident("else"))
					{
						if (skip_level == 1)
						{
							if (!if_done[if_level])
								skip_level = 0;
						}
					}
					else if (is_ident("elif"))
					{
						if (skip_level == 1)
						{
							if (!if_done[if_level])
							{
								if (parse_cond())
								{
									skip_level = 0;
									if_done[if_level] = true;
								}
							}
						}	
					}
				}
				else
				{
					if (is_ident("ifdef"))
					{
						while (*s == ' ' || *s == '\t') s++;
						parse_token();
						if (kind != 'i')
						{
							fprintf(stderr, "%s:%d Expect ident after #ifdef, not '%s'\n", name, line_nr, token);
							exit(1);
						}
						if_level++;
						if (get_env(token) != 0)
						{
							if_done[if_level] = true;
						}
						else
						{
							if_done[if_level] = false;
							skip_level = 1;
						} 
					}
					else if (is_ident("ifndef"))
					{
						while (*s == ' ' || *s == '\t') s++;
						parse_token();
						if (kind != 'i')
						{
							fprintf(stderr, "%s:%d Expect ident after #ifndef, not '%s'\n", name, line_nr, token);
							exit(1);
						}
						if_level++;
						if (get_env(token) == 0)
						{
							if_done[if_level] = true;
						}
						else
						{
							if_done[if_level] = false;
							skip_level = 1;
						} 
					}
					else if (is_ident("if"))
					{
						if_level++;
						if (parse_cond())
						{
							if_done[if_level] = true;
						}
						else
						{
							if_done[if_level] = false;
							skip_level = 1;
						} 
					}
					else if (is_ident("else"))
					{
						if (if_level == -1)
						{
							fprintf(stderr, "%s:%d unexpected #else\n", name, line_nr);
							exit(1);
						}
						skip_level = 1;
					}
					else if (is_ident("elif"))
					{
						if (if_level == -1)
						{
							fprintf(stderr, "%s:%d unexpected #elif\n", name, line_nr);
							exit(1);
						}
						skip_level = 1;
					}
					else if (is_ident("endif"))
					{
						if (if_level == -1)
						{
							fprintf(stderr, "%s:%d unexpected #endif\n", name, line_nr);
							exit(1);
						}
						if_level--;
					}
					else if (is_ident("define"))
					{
						while (*s == ' ' || *s == '\t') s++;
						if (kind != 'i')
						{
							fprintf(stderr, "%s:%d Expect ident after #define, not '%s'\n", name, line_nr, token);
							exit(1);
						}
						parse_token();
						// remove spaces, but leave one space before ( to make a differentiate between a define with argument and without.
						while ((*s == ' ' || *s == '\t') && s[1] != '(')
							s++;
						fill_large_buffer();
						set_env(token, large_buffer);
					}
					else if (is_ident("undef"))
					{
						while (*s == ' ' || *s == '\t') s++;
						parse_token();
						if (kind != 'i')
						{
							fprintf(stderr, "%s:%d Expect ident after #define, not '%s'\n", name, line_nr, token);
							exit(1);
						}
						del_env(token);
					}
					else if (is_ident("include"))
					{
						while (*s == ' ' || *s == '\t') s++;
						if (*s == '"')
						{
							s++;
							int i = 0;
							while (*s != '\0' && *s != '\n' && *s != '"')
								token[i++] = *s++;
							token[i] = '\0';
							fprintf(stderr, "Including %s\n", token);
							Parser include_parser(token, fout);
							include_parser.parse();
							fprintf(stderr, "Finish including %s\n", token);
						}
					}
					else
					{
						fprintf(stderr, "%s:%d Unknown macro '%s' after '#'\n", name, line_nr, token);
						exit(1);
					}
				}
			}
			else if (skip_level > 0)
			{
				fprintf(stderr, "SKIP %s", line);
			}
			else
			{
				s = line;
				for (;;)
				{
					while (*s == ' ' || *s == '\t')
						fputc(*s++, fout);
					if (*s == '\0' || *s == '\n' || (s[0] == '/' && s[1] == '/'))
					{
						fputc('\n', fout);
						break;
					}
					if (s[0] == '/' && s[1] == '*')
					{
						s += 2;
						for (;;)
						{
							if (*s == '\0' || *s == '\n')
							{
								if (!fgets(line, 400, f))
								{
									fprintf(stderr, "Unterminated /* comment\n");
									exit(1);
								}
								line_nr++;
								s = line;
							}
							else if (s[0] == '*' && s[1] == '/')
							{
								s += 2;
								fputc(' ', fout);
								break;
							}
							else
								s++;
						}
					}
					else if (*s < ' ')
						s++;
					else
					{
						parse_token();
						if (kind == 'i')
						{
							const char *value = get_env(token);
							if (value == 0)
								fprintf(fout, "%s", token);
							else
							{
								expand(token, value);
							}
						}
						else
							fprintf(fout, "%s", token);
					}
				}
			}
		}
	}
private:
	FILE *f;
	char line[401];
	const char *s;
	char token[400];
	char kind;
	long int int_value;
	bool if_done[40];
	int if_level;
	int skip_level;
	
	void parse_token()
	{
		int i = 0;
		
		if (*s == '\0' || *s == '\n')
			kind = ' ';
		else if (('a' <= *s && *s <= 'z') || ('A' <= *s && *s <= 'Z') || *s == '_')
		{
			kind = 'i';
			token[i++] = *s++;
			while (('a' <= *s && *s <= 'z') || ('A' <= *s && *s <= 'Z') || ('0' <= *s && *s <= '9') ||*s == '_')
				token[i++] = *s++;
		}
		else if (('0' <= *s && *s <= '9') || (*s == '-' && ('0' <= s[1] && s[1] <= '9')))
		{
			kind = '0';
			int sign = 1;
			int_value = 0;
			if (*s == '-')
			{
				sign = -1;
				token[i++] = *s++;
			}
			if (*s == '0')
			{
				token[i++] = *s++;
				if (*s == 'x')
				{
					token[i++] = *s++;
					for (;;)
					{
						if ('0' <= *s && *s <= '9')
							int_value = 16 * int_value + *s - '0';
						else if ('a' <= *s && *s <= 'f')
							int_value = 16 * int_value + *s - 'a' + 10;
						else if ('A' <= *s && *s <= 'F')
							int_value = 16 * int_value + *s - 'A' + 10;
						else
							break;
						token[i++] = *s++;
					}
				}
				else
				{
					while ('0' <= *s && *s <= '7')
					{
						int_value = 8 * int_value + *s - '0';
						token[i++] = *s++;
					}
				}
			}
			else
			{
				while ('0' <= *s && *s <= '9')
				{
					int_value = 10 * int_value + *s - '0';
					token[i++] = *s++;
				}
			}
			int_value *= sign;
		}
		else if (*s == '"' || *s == '\'')
		{
			kind = '"';
			char quote = *s;
			token[i++] = *s++;
			while (*s != '\0' && *s != '\n' && *s != quote)
			{
				if (*s == '\\')
					token[i++] = *s++;
				token[i++] = *s++;
			}
			if (*s == quote)
				token[i++] = *s++;
		}
		else
		{
			kind = '+';
			token[i++] = *s++;
		}
		token[i] = '\0';
	}
	
	void fill_large_buffer()
	{
		int i = 0;
		while (*s != '\0' && *s != '\n')
		{
			if (s[0] == '\\' && s[1] == '\n')
			{
				if (!fgets(line, 400, f))
				{
					printf("%s:%d No more input after \\-terminated line", name, line_nr);
					exit(1);
				}
				line_nr++;
				s = line;
			}
			else if (s[0] == '/')
			{
				if (s[1] == '/')
					break;
				else if (s[1] == '*')
				{
					s += 2;
					for (;;)
					{
						if (*s == '\0' || *s == '\n')
						{
							if (!fgets(line, 400, f))
							{
								fprintf(stderr, "Unterminated /* comment\n");
								exit(1);
							}
							line_nr++;
							s = line;
						}
						else if (s[0] == '*' && s[1] == '/')
						{
							s += 2;
							large_buffer[i++] = ' ';
							break;
						}
						else
							s++;
					}
				}
				else
					large_buffer[i++] = *s++;
			}
			else if (*s == ' ' || *s == '\t')
			{
				s++;
				while (*s == ' ' || *s == '\t') s++;
				large_buffer[i++] = ' ';
			}
			else if (*s > ' ')
				large_buffer[i++] = *s++;
		}
		// Remove trailing space
		while (i > 0 && (large_buffer[i-1] == ' ' || large_buffer[i-1] == '\t'))
			i--;
		large_buffer[i] = '\0';
	}
	
	int parse_primary_expr()
	{
		fprintf(stderr, "Parse_primary '%s'\n", s);
		while (*s == ' ' || *s == '\t') s++;
		if (*s == '(')
		{
			s++;
			while (*s == ' ' || *s == '\t') s++;
			int value = parse_or_expr();
			while (*s == ' ' || *s == '\t') s++;
			if (*s != ')')
			{
				fprintf(stderr, "%s:%d expecting ) at %s", name, line_nr, s);
				exit(1);
			}
			s++;
			return value;
		}
	
		parse_token();
		fprintf(stderr, "Token |%s|\n", token);
		if (is_ident("defined"))
		{
			while (*s == ' ' || *s == '\t') s++;
			if (*s == '(')
			{
				s++;
				while (*s == ' ' || *s == '\t') s++;
				parse_token();
				fprintf(stderr, "Token id |%s|", token);
				while (*s == ' ' || *s == '\t') s++;
				if (*s != ')')
				{
					fprintf(stderr, "%s:%d expecting ) at %s", name, line_nr, s);
					exit(1);
				}
				s++;
			}
			else
				parse_token();
			fprintf(stderr, "Check existing '%s'\n", token);
			return get_env(token) != 0 ? 1 : 0;
		}
		
		const char *save_s = s;
		for (;;)
		{
			if (kind == '0')
			{
				s = save_s;
				return int_value;
			}
			else if (kind == 'i')
			{
				const char *value = get_env(token);
				if (value == 0)
				{
					s = save_s;
					return 0;
				}
				fprintf(stderr, "Expanded '%s' to '%s'\n", token, value);
				s = value;
				while (*s == ' ') s++;
				parse_token();
			}
			else
			{
				fprintf(stderr, "%s.%d unknown kind '%c' for '%s' at '%s' line %s\n", name, line_nr, kind, token, s, line);
				exit(1);
			}
		}
		s = save_s;
		return 0;
	}
	
	int parse_unary_expr()
	{
		fprintf(stderr, "Parse unary '%s'\n", s);
		while (*s == ' ' || *s == '\t') s++;
		if (*s == '!')
		{
			s++;
			return parse_primary_expr() == 0 ? 1 : 0;
		}
		return parse_primary_expr();
	}
	
	int parse_compare_expr()
	{
		fprintf(stderr, "Parse compare '%s'\n", s);
		int value = parse_unary_expr();
		while (*s == ' ' || *s == '\t') s++;
		if (s[0] == '=' && s[1] == '=')
		{
			s += 2;
			int rhs = parse_unary_expr();
			return value == rhs ? 1 : 0;
		}
		else if (s[0] == '!' && s[1] == '=')
		{
			s += 2;
			int rhs = parse_unary_expr();
			return value != rhs ? 1 : 0;
		}
		return value;
	} 
	
	int parse_and_expr()
	{
		fprintf(stderr, "Parse and '%s'\n", s);
		int value = parse_compare_expr();
		for (;;)
		{
			while (*s == ' ' || *s == '\t') s++;
			if (s[0] == '&' && s[1] == '&')
			{
				s += 2;
				if (parse_compare_expr() == 0)
					value = 0;
			}
			else
				break;
		}
		return value;
	}

	int parse_or_expr()
	{
		fprintf(stderr, "Parse or '%s'\n", s);
		int value = parse_and_expr();
		for (;;)
		{
			while (*s == ' ' || *s == '\t') s++;
			if (s[0] == '|' && s[1] == '|')
			{
				s += 2;
				if (parse_and_expr() != 0)
					value = true;
			}
			else
				break;
		}
		return value;
	}
	
	bool parse_cond()
	{
		fill_large_buffer();
		
		s = large_buffer;
		int value = parse_or_expr();
		if (*s != '\0')
		{
			fprintf(stderr, "%s:%d unparsed condition at %s", name, line_nr, s);
			exit(1);
		}
		fprintf(stderr, "Cond = %d\n", value);
		return value != 0;
	}
	
	bool is_ident(const char *name) { return kind == 'i' && strcmp(token, name) == 0; }

	void expand(const char *symbol, const char *value)
	{
		char expanded_result[500];

		const char *save_s = s;
		if (*value == '(')
		{
			// The symbol is a macro with arguments. First checked of the input start with a bracker.
			while (*s == ' ' || *s == '\t') s++;
			if (*s != '(')
			{
				fprintf(stderr, "%s:%d No expansion of %s\n", name, line_nr, symbol);
				fprintf(fout, "%s ", symbol);
				return;
			}
			
			// Now parse the parameters (assuming that there are not more than 10)
			//fprintf(fout, "\n/* Expand %s '%s' on '%s' */", symbol, value, s);
			const char *params[10];
			int nr_params = 0;
			const char *v = value + 1;
			for (;nr_params < 9;)
			{
				while (*v == ' ') v++;
				if (*v == ')')
					break;
				params[nr_params++] = v;
				while (*v != '\0' && *v != ' ' && *v != ',' && *v != ')')
					v++;
				if (*v == ' ') v++;
				if (*v == ',') v++;
			}
			if (*v == ')')
				v++;
			//fprintf(fout, " number params = %d\n", nr_params);

			// Now parse the argument, taking into account the usage of '(' and ')' in the arguments
			s++;
			const char *args_start[10];
			const char *args_end[10];
			int nr_args = 0;
			while (*s != '\0' && nr_args < 10)
			{
				while (*s == ' ') s++;
				args_start[nr_args] = s;
				args_end[nr_args] = 0;
				int skip = 0;
				while (*s != '\0')
				{
					if (*s == '(')
						skip++;
					else if (skip > 0)
					{
						if (*s == ')')
							skip--;
					}
					else if (*s == ',' || *s == ')')
						break;
					args_end[nr_args] = *s == ' ' ? s : 0;
					s++;
				}
				if (args_end[nr_args] == 0)
					args_end[nr_args] = s;
				nr_args++;
				if (*s == ',')
					s++;
				else
					break;
			}
			if (*s == ')')
				s++;
			
			// The number of arguments should match the number of parameters
			if (nr_args != nr_params)
			{
				fprintf(stderr, "%s:%d Arg %d params %d: %s\n", name, line_nr, nr_args, nr_params, line);
				exit(1);
			}
			/*
			for (int i = 0; i < nr_params; i++)
			{
				fprintf(fout, " param %d: '", i);
				const char *p = params[i];
				while (*p != ' ' && *p != ',' && *p != ')')
					fprintf(fout, "%c", *p++);
				fprintf(fout, " = '");
				for (const char *a = args_start[i]; a < args_end[i]; a++)
					fprintf(fout, "%c", *a);
				fprintf(fout, "'\n");
			}
			*/
			
			// Using the arguments and parameters, expand the value of the symbol
			char *r = expanded_result;
			int k = 0;
			save_s = s;
			s = v;
			bool need_space = false;
			while (*s != '\0')
			{
				while (*s == ' ' || *s == '\t') s++;
				
				// A single '#' means that the next ident should be stringified (after possible substituion)
				bool stringify = false;
				if (*s == '#')
				{
					s++;
					stringify = true;
				}
				
				parse_token();
				if (kind == 'i')
				{
					const char *start = token;
					const char *end = token + strlen(token);
					for (int i = 0; i < nr_params; i++)
					{
						int j = 0;
						const char *p = params[i];
						while (token[j] == *p)
						{
							j++;
							p++;
						}
						if (token[j] == '\0' && (*p == ' ' || *p == ',' || *p == ')'))
						{
							// The ident does match one of the parametsrs
							start = args_start[i];
							end = args_end[i];
							break;
						}
					}
					if (stringify)
					{
						*r++ = '\"';
						for(; start < end; start++)
						{
							if (*start == '"' || *start == '\\')
								*r++ = '\\';
							*r++ = *start;
						}
						*r++ = '"';
					}
					else
					{
						if (need_space)
							*r++ = ' ';
						for(; start < end; start++)
							*r++ = *start;
					}
				}
				else if (kind == '0')
				{
					if (need_space)
						*r++ = ' ';
					strcpy(r, token);
					r += strlen(r);
				}
				else
				{
					need_space = false;
					strcpy(r, token);
					r += strlen(r);
				}

				// If it is followed by a double '#', it means that there should be no space
				while (*s == ' ' || *s == '\t') s++;
				if (s[0] == '#' && s[1] == '#')
				{
					s += 2;
					while (*s == ' ' || *s == '\t') s++;
					need_space = false;
				}
				else
					need_space = true;
			}
			*r = '\0';
			//fprintf(fout, "\n/* Expanded: %s */", expanded_result);
		}
		else
			strcpy(expanded_result, value);
		
		// Now expand the expanded result
		s = expanded_result;
		for (bool go = true; go;)
		{
			for (; *s == ' ' || *s == '\t'; s++)
				fprintf(fout, "%c", *s);
			if (*s == '\0')
				break;
			parse_token();
			if (kind == 'i')
			{
				const char *val = get_env(token);
				if (val == 0)
					fprintf(fout, "%s", token);
				else
				{
					for (; *s == ' ' || *s == '\t'; s++)
						fprintf(fout, "%c", *s);
					if (*s == '\0')
					{
						s = save_s;
						save_s = 0;
						go = false;
					}
					expand(token, val);
				}
			}
			else
				fprintf(fout, "%s", token);
		}
		if (save_s != 0)
			s = save_s;
	}
};

int main(int argc, char *argv[])
{
	set_env("ONE_SOURCE", "1");
	set_env("TCC_VERSION", "\"1.0\"");
	
	Parser parser("tcc.c", stdout);
	parser.parse();
	
	return 0;
}