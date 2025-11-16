/*	Stack_C: Emulator for the Stack C language

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
- http://ref.x86asm.net/geek.html
- https://www.felixcloutier.com/x86/
- https://faculty.nps.edu/cseagle/assembly/sys_call.html
- https://defuse.ca/online-x86-assembler.htm

*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

// Constants

#define MAX_TOKEN_LENGTH 8000
#define MAX_NR_VARIABLES 4000
#define MAX_NR_STATICS 10
#define MAX_VARIABLE_LENGTH 50
#define MAX_NESTING 100

// Boolean definition

#define TRUE 1
#define FALSE 0
typedef int bool;

// 

char *copystr(const char *s)
{
	size_t len = strlen(s);
	char *result = (char*)malloc(len + 1);
	strcpy(result, s);
	return result;
}

// Global variables

FILE *fin;
FILE *fout;
FILE *ferr;
char cur_char = '\0';
bool at_end_of_line = FALSE;
int cur_char_line = 1;
int cur_char_column = 1;

// Read charachter

void read_char(void)
{
	if (feof(fin))
		cur_char = '\0';
	else
	{
		if (at_end_of_line)
		{
			at_end_of_line = FALSE;
			cur_char_line++;
			cur_char_column = 0;
		}
		cur_char = fgetc(fin);
		if (feof(fin))
			cur_char = '\0';
		else
		{
			cur_char_column++;
			if (cur_char == '\n')
				at_end_of_line = TRUE;
			else if (cur_char == '\t')
				cur_char_column += 4 - ((cur_char_column - 1) % 4);
		}
	}
}

char sym;
char token[MAX_TOKEN_LENGTH+1];
int token_len = 0;
int int_value;
int cur_line = 0;
int cur_column = 0;

typedef struct
{
	const char *name;
	char sym;
} Mapping;
#define NR_KEYWORDS 12
Mapping keywords[NR_KEYWORDS] = {
	{ "void",		'F' },
	{ "const",		'C' },
	{ "int",		'V' },
	{ "do",			'L' },
	{ "break",		'B' },
	{ "continue",	'D' },
	{ "if",			'I' },
	{ "else" ,		'E' },
	{ "return",		'R' },
	{ "goto",		'G' },
	{ "static",     'S' },
	{ "char",       'K' }
};

#define SYMBOL(X) ('a' + (X))

#define NR_SYMBOLS 21
Mapping symbols[NR_SYMBOLS] = {
#define SYM_REV_ASS SYMBOL(0)
	{ "=:",         SYM_REV_ASS },
#define SYM_GET_BYTE SYMBOL(1)
	{ "?1",         SYM_GET_BYTE },
#define SYM_ASS_BYTE SYMBOL(2)
	{ "=1",         SYM_ASS_BYTE },
#define SYM_CALL SYMBOL(3)
	{ "()",         SYM_CALL },
#define SYM_DIV_SIGNED SYMBOL(4)
	{ "/s",         SYM_DIV_SIGNED },
#define SYM_MOD_SIGNED SYMBOL(5)
	{ "%s",         SYM_MOD_SIGNED },
#define SYM_EQ SYMBOL(6)
	{ "==",         SYM_EQ },
#define SYM_NE SYMBOL(7)
	{ "!=",         SYM_NE },
#define SYM_LE SYMBOL(8)
	{ "<=",         SYM_LE },
#define SYM_GE SYMBOL(9)
	{ ">=",         SYM_GE },
#define SYM_LT_SIGNED SYMBOL(10)
	{ "<s",         SYM_LT_SIGNED },
#define SYM_LE_SIGNED SYMBOL(11)
	{ "<=s",         SYM_LE_SIGNED },
#define SYM_GT_SIGNED SYMBOL(12)
	{ ">s",         SYM_GT_SIGNED },
#define SYM_GE_SIGNED SYMBOL(13)
	{ ">=s",         SYM_GE_SIGNED },
#define SYM_SHL SYMBOL(14)
	{ "<<",         SYM_SHL },
#define SYM_SHR SYMBOL(15)
	{ ">>",         SYM_SHR },
#define SYM_LOG_AND SYMBOL(16)
	{ "&&",         SYM_LOG_AND },
#define SYM_LOG_OR SYMBOL(17)
	{ "||",         SYM_LOG_OR },
#define SYM_ARROW SYMBOL(18)
	{ "->",         SYM_ARROW },
#define SYM_SWAP SYMBOL(19)
	{ "><",         SYM_SWAP },
#define SYM_SUB_PTRS SYMBOL(20)
	{ "-p",         SYM_SUB_PTRS }
};

int error = 0;

bool opt_trace_parsing = FALSE;

char *c_filename = NULL;
int c_line = 0;

void get_token(void)
{
	token_len = 0;
	// Skip white spaces, but echo comments starting with # till end of the line
	while ((cur_char != '\0' && cur_char <= ' ') || cur_char == '#')
	{
		if (cur_char == '#')
		{
			char buffer[100];
			int i = 0;
			for (;;)
			{
				//fputc(cur_char, fout);
				read_char();
				if (cur_char == '\0' || cur_char == '\n')
					break;
				if (i < 99)
					buffer[i++] = cur_char;
			}
			if (cur_char == '\n')
				read_char();
			buffer[i] = '\0';

			if (buffer[0] == ' ')
			{
				i = 1;
				while (buffer[i] != '\0' && buffer[i] != ' ')
					i++;
				if (i > 1 && buffer[i] == ' ')
				{
					buffer[i] = '\0';
					i++;
					int l = 0;
					while ('0' <= buffer[i] && buffer[i] <= '9')
						l = 10 * l + buffer[i++] - '0';
					if (l > 0 && buffer[i] == 0)
					{
						c_filename = copystr(buffer + 1);
						c_line = l;
					}
				}
			}
			//fputc('\n', fout);
		}
		else
		{
			if (cur_char == '\n')
				c_line++;
			read_char();
		}
	}

	// Check for end of file
	if (cur_char == '\0')
	{
		sym = '\0';
		return;
	}

	// Store starting position
	cur_line = cur_char_line;
	cur_column = cur_char_column;

	// Check for identifier
	if (('a' <= cur_char && cur_char <= 'z') || ('A' <= cur_char && cur_char <= 'Z') || cur_char == '_')
	{
		sym = 'A';
		do
		{
			token[token_len++] = cur_char;
			read_char();
		}
		while (('a' <= cur_char && cur_char <= 'z') || ('A' <= cur_char && cur_char <= 'Z') || cur_char == '_' || ('0' <= cur_char && cur_char <= '9'));
		token[token_len] = '\0';

		if (token_len > MAX_VARIABLE_LENGTH)
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
		if (opt_trace_parsing) printf("%d.%d get_token %s %c\n", cur_line, cur_column, token, sym);
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
			if (token_len > MAX_TOKEN_LENGTH)
			{
				fprintf(ferr, "ERROR %d: String longer than %d characters\n", cur_line, MAX_TOKEN_LENGTH);
				error = 1;
				sym = '\0';
				if (opt_trace_parsing) printf("%d.%d get_token %s %c\n", cur_line, cur_column, token, sym);
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
				token[token_len++] = cur_char;
			}
			else
				token[token_len++] = cur_char;
		}
		token[token_len] = '\0';
		if (opt_trace_parsing) printf("%d.%d get_token %s %c\n", cur_line, cur_column, token, sym);
		return;
	}

	// Check for number or '->' symbol
	if (('0' <= cur_char && cur_char <= '9') || cur_char == '-')
	{
		token[token_len++] = cur_char;
		int sign = 1;
		if (cur_char == '-')
		{
			sign = -1;
			read_char();
			// check of '->' symbol
			if (cur_char == '>')
			{
				sym = SYM_ARROW;
				token[token_len++] = cur_char;
				token[token_len] = '\0';
				read_char();
				return;
			}
			if (cur_char == 'p')
			{
				sym = SYM_SUB_PTRS;
				token[token_len++] = cur_char;
				token[token_len] = '\0';
				read_char();
				return;
			}
			if (cur_char <= ' ')
			{
				sym = '-';
				token[token_len] = '\0';
				if (opt_trace_parsing) printf("%d.%d get_token %s %c\n", cur_line, cur_column, token, sym);
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
			if (opt_trace_parsing) {
				sprintf(token, "%d", int_value);
				printf("%d.%d get_token %s %c\n", cur_line, cur_column, token, sym);
			}
			return; 
		}
	}

	// Label start
	if (cur_char == ':')
	{
		sym = ':';
		token[1] = '\0';
		token_len = 1;
		read_char();
		if (opt_trace_parsing) printf("%d.%d get_token %s %c\n", cur_line, cur_column, token, sym);
		return;
	}

	// Parse symbol till next white space
	sym = ' ';
	do
	{
		token[token_len++] = cur_char;
		read_char();
	}
	while (cur_char > ' ');

	token[token_len] = '\0';
	if (token_len == 1)
		sym = token[0];
	else
		for (int i = 0; i < NR_SYMBOLS; i++)
			if (strcmp(token, symbols[i].name) == 0)
			{
				sym = symbols[i].sym;
				if (opt_trace_parsing) printf("%d.%d get_token %s %c\n", cur_line, cur_column, token, sym);
				return;
			}
	if (opt_trace_parsing) printf("%d.%d get_token %s %c\n", cur_line, cur_column, token, sym);
}

// Typedefs

typedef struct command_s *command_p;
typedef struct memory_s *memory_p;
typedef struct cell_s cell_t, *cell_p;
typedef struct function_s *function_p;

// Identifiers

typedef struct
{
	char type;   // 'F': Function, 'C': constant, 'M': global variable, 'L': local variable
	char name[MAX_VARIABLE_LENGTH+1];
	int pos;     // position for local variable
	int size;    // size for variable
	int value;   // value for constant, nr for static
	function_p function;
	memory_p memory;
} ident_t, *ident_p;

ident_t idents[MAX_NR_VARIABLES];
int nr_idents = 0;
int pos = 0;

//

command_p *ref_command = NULL;

// String constants

typedef struct string_s *string_p;
struct string_s
{
	char *value;
	int length;
	string_p next;
};
string_p strings = 0;

string_p unique_string(const char *s, int length)
{
	//printf("# nr_for_string %s\n", s);
	int nr = 0;
	string_p *ref_string = &strings;
	for (; (*ref_string) != 0; ref_string = &(*ref_string)->next, nr++)
		if ((*ref_string)->length == length && memcmp((*ref_string)->value, s, length) == 0)
			return (*ref_string);
	(*ref_string) = (string_p)malloc(sizeof(struct string_s));
	(*ref_string)->value = (char*)malloc(length + 1);
	memcpy((*ref_string)->value, s, length + 1);
	(*ref_string)->length = length;
	(*ref_string)->next = 0;
	return (*ref_string);
}

int nesting_depth = 0;
int nesting_nr_vars[MAX_NESTING];
int nesting_pos[MAX_NESTING];
char nesting_type[MAX_NESTING];
int nesting_id[MAX_NESTING];
command_p nesting_command[MAX_NESTING];

struct command_s
{
	char sym;
	int int_value;
	union {
		string_p str;
		command_p jump_command;
		memory_p memory;
		function_p function;
	};
	int line;
	int column;
	char *c_filename;
	int c_line;
	command_p next;
};

struct function_s
{
	void (*sys_function)(void);
	command_p commands;
	int max_locals_depth;
	ident_p ident;
};

ident_p add_function(const char *name)
{
	ident_p ident = &idents[nr_idents++];
	ident->type = 'F';
	strcpy(ident->name, name);
	ident->function = (function_p)malloc(sizeof(struct function_s));
	ident->function->sys_function = 0;
	ident->function->commands = NULL;
	ident->function->max_locals_depth = 0;
	ident->function->ident = ident;
	ident->memory = NULL;
	return ident;
} 

typedef struct jump_command_s *jump_command_p;
struct jump_command_s
{
	command_p command;
	jump_command_p next;
};

typedef struct label_s *label_p;
struct label_s
{
	char *name;
	bool defined;
	command_p jump_to;            // The command following the label
	jump_command_p jump_commands; // List of commands that jump to this label
	label_p for_next_command;     // Used in list for labels_next_command
	label_p prev;                 // Previous label in list of labels with a function
};

label_p func_labels = NULL;         // List of all labels in current function in reverse order
label_p labels_next_command = NULL; // List of labels (via for_next_command) for the next command

label_p find_label(const char *fmt, ...)
{
	static char name[300];
	va_list args;
   	va_start(args, fmt);
	vsnprintf(name, 299, fmt, args);
	va_end(args);
	name[299] = '\0';

	for (label_p label = func_labels; label != NULL; label = label->prev)
		if (strcmp(label->name, name) == 0)
			return label;
	label_p label = (label_p)malloc(sizeof(struct label_s));
	label->name = copystr(name);
	label->defined = FALSE;
	label->jump_to = NULL;
	label->jump_commands = NULL;
	label->for_next_command = NULL;
	label->prev = func_labels;
	func_labels = label;
	return label;
}

bool opt_trace_labels = FALSE;

void add_label(label_p label)
{
	if (opt_trace_labels) printf("## Add label %s\n", label->name);
	if (label->defined)
	{
		fprintf(ferr, "Error %d.%d: Label %s is defined more than once", cur_line, cur_column, label->name);
		exit(1);
	}
	label->defined = TRUE;
	label->for_next_command = labels_next_command;
	labels_next_command = label;
}

command_p add_command(char sym)
{
	command_p command = (command_p)malloc(sizeof(struct command_s));
	command->sym = sym;
	command->int_value = 0;
	command->str = NULL;
	command->line = cur_line;
	command->column = cur_column;
	command->c_filename = c_filename;
	command->c_line = c_line;
	command->next = NULL;
	if (ref_command != NULL)
	{
		*ref_command = command;
		ref_command = &command->next;
	}
	// If there are labels for this command, update these labels to point to this command
	for (; labels_next_command != NULL; labels_next_command = labels_next_command->for_next_command)
	{
		labels_next_command->jump_to = command;
		// For the goto statements that use the label, update the jump_command of these to point to this command
		for (jump_command_p jump_command = labels_next_command->jump_commands; jump_command != NULL; jump_command = jump_command->next)
		{
			jump_command->command->jump_command = command;
			if (opt_trace_labels) printf("Command %d.%d jump to %d.%d\n", jump_command->command->line, jump_command->command->column, command->line, command->column);
		}
	}
	return command;
}

void add_jump_command(char sym, label_p label)
{
	if (opt_trace_labels) printf("## Add jump to label %s (%sdefined)\n", label->name, label->defined ? "" : "not ");
	command_p command = add_command(sym);
	if (label->jump_to != NULL)
	{
		// The label already has been defined: set the jump_to the command following the label 
		command->jump_command = label->jump_to;
		if (opt_trace_labels) printf("Comaand %d.%d jump to %d.%d\n", command->line, command->column, command->jump_command->line, command->jump_command->column);
	}
	else
	{
		// The label is not yet defined: Add the command to the list of command that jump to the label
		jump_command_p jump_command = (jump_command_p)malloc(sizeof(struct jump_command_s));
		jump_command->command = command;
		jump_command->next = label->jump_commands;
		label->jump_commands = jump_command;
	}
}

struct memory_s
{
	int nr_cells;
	cell_p cells;
	const char *name;
	int line;
	int column;
};

typedef enum {
	C_UNDEFINED,
	C_VALUE,
	C_STRING,
	C_GLOBAL,
	C_LOCAL,
	C_FUNCTION,
} cell_kind_e;

struct cell_s
{
	cell_kind_e kind;
	int int_value; // offset for memory
	int locals_offset;
	union {
		memory_p memory;
		function_p function;
		string_p str;
	};
	command_p command;
};

const char* cell_kind_name(cell_p cell)
{
	switch (cell->kind)
	{
		case C_UNDEFINED: return "undefined";
		case C_VALUE: return "value";
		case C_STRING: return "string";
		case C_GLOBAL: return "global";
		case C_LOCAL: return "local";
		case C_FUNCTION: return "function";
	}
	return "<incorrect>";
}

char function_name[MAX_VARIABLE_LENGTH+1];
int id = 0;

#define MAX_VALUE_DEPTH 10000
cell_t value_stack[MAX_VALUE_DEPTH];
int value_depth = 0;
cell_p top_value;

#define MAX_LOCALS_DEPTH 10000
cell_t locals_stack[MAX_LOCALS_DEPTH];
int locals_offset = 0;

#define MAX_FUNCTION_DEPTH 1000
struct 
{
	function_p function;
	command_p command;
} function_stack[MAX_FUNCTION_DEPTH];
int function_depth = 0;
function_p cur_function = NULL;

command_p cur_command = NULL;

const char *command_name(command_p command)
{
	if (command == NULL)
		return "<null>";
	char sym = cur_command->sym;
	if (SYMBOL(0) <= sym && sym < SYMBOL(20))
		return symbols[sym - SYMBOL(0)].name;
	if (sym == 'M')
		return "global var";
	if ('A' <= sym && sym <= 'Z')
	{
		for (int i = 0; i < NR_KEYWORDS; i++)
			if (keywords[i].sym == sym)
				return keywords[i].name;
	}
	static char name[2];
	name[0] = sym;
	name[1] = '\0';
	return name;
}

void print_value_stack(FILE *f)
{
	for (int i = 1; i <= value_depth; i++)
	{
		if (i >= 2)
			fprintf(f, ",");
		fprintf(f, " %s ", cell_kind_name(&value_stack[i]));
		if (value_stack[i].kind == C_GLOBAL || value_stack[i].kind == C_LOCAL)
		{
			memory_p memory = value_stack[i].memory;
			fprintf(f, "%s %d.%d", memory->name, memory->line, memory->column);
			if (value_stack[i].int_value != 0)
				fprintf(f, "[%d]", value_stack[i].int_value);
			if (value_stack[i].kind == C_LOCAL)
				fprintf(f, "(%d)", value_stack[i].locals_offset);
		}
		else
		{
			if (value_stack[i].kind == C_VALUE)
				fprintf(f, "%d ", value_stack[i].int_value);
			if (value_stack[i].command != NULL)
				fprintf(f, " %d.%d", value_stack[i].command->line, value_stack[i].command->column);
			else
				fprintf(f, " ?.?");
			if (value_stack[i].kind == C_STRING)
				fprintf(f, " '%s'", value_stack[i].str->value);
		}
	}
	fprintf(f, ")\n");
}

void report_error(const char *fmt, ...)
{
	for (int i = 0; i < function_depth; i++)
		if (function_stack[i].command->c_filename != NULL)
			fprintf(ferr, "At %s:%d (%d.%d) in function %s\n",
				function_stack[i].command->c_filename, function_stack[i].command->c_line, 
				function_stack[i].command->line, function_stack[i].command->column, function_stack[i].function->ident->name);
		else
			fprintf(ferr, "At %d.%d in function %s\n",
				function_stack[i].command->line, function_stack[i].command->column, function_stack[i].function->ident->name);
	if (cur_command == NULL)
		fprintf(ferr, "In function %s\n", cur_function->ident->name);
	else if (cur_command->c_filename != NULL)
		fprintf(ferr, "At %s:%d (%d.%d) in function %s\n",
			cur_command->c_filename, cur_command->c_line, 
			cur_command->line, cur_command->column, cur_function->ident->name);
	else
		fprintf(ferr, "At %d.%d in function %s\n",
			cur_command->line, cur_command->column, cur_function->ident->name);
	fprintf(ferr, "Stack: ");
	print_value_stack(ferr);
	static char message[300];
	va_list args;
   	va_start(args, fmt);
	vsnprintf(message, 299, fmt, args);
	va_end(args);
	message[299] = '\0';
	fprintf(ferr, "Error: %s\n", message);
	exit(-1);
}

void report_warning(const char *fmt, ...)
{
	//for (int i = 0; i < function_depth; i++)
	//	fprintf(ferr, "At %d.%d in function %s\n",
	//		function_stack[i].command->line, function_stack[i].command->column, function_stack[i].function->ident->name);
	fprintf(ferr, "In function %s\n", cur_function->ident->name);
	fprintf(ferr, "Stack: ");
	print_value_stack(ferr);
	static char message[300];
	va_list args;
   	va_start(args, fmt);
	vsnprintf(message, 299, fmt, args);
	va_end(args);
	message[299] = '\0';
	if (cur_command != NULL)
		fprintf(ferr, "Warning %d.%d %s:%d [%s]: %s\n", cur_command->line, cur_command->column, cur_command->c_filename, cur_command->c_line, command_name(cur_command), message);
	else
		fprintf(ferr, "Warning: %s\n", message);
}

void pop(void)
{
	if (value_depth <= 0)
		report_error("Stack is empty");
	top_value = &value_stack[--value_depth];
}

int get_top_value(void)
{
	if (value_depth <= 0)
		report_error("Stack is empty");
	if (top_value->kind != C_VALUE)
		report_error("Stack does not contain value but %s", cell_kind_name(&value_stack[value_depth]));
	return top_value->int_value;
}

int pop_value(void)
{
	int value = get_top_value();
	pop();
	return value;
}

bool get_top_bool(void)
{
	if (value_depth <= 0)
		report_error("Stack is empty");
	if (top_value->kind == C_VALUE)
		return top_value->int_value != 0;
	if (top_value->kind == C_FUNCTION)
		report_error("Stack does not contains undefined value");
	return TRUE;
}

bool pop_bool(void)
{
	bool value = get_top_bool();
	pop();
	return value;
}

void push(char kind)
{
	top_value = &value_stack[++value_depth];
	top_value->kind = kind;
	top_value->command = cur_command;
}

void push_value(int value)
{
	if (value_depth + 1 >= MAX_VALUE_DEPTH)
		report_error("Stack overflow");
	top_value = &value_stack[++value_depth];
	top_value->kind = C_VALUE;
	top_value->int_value = value;
	top_value->command = cur_command;
}

void check_stack(int depth)
{
	if (value_depth < depth)
		report_error("Stack only has %d values but command requires %d", value_depth, depth);
}

cell_p deref(cell_p cell, bool is_word)
{
	if (cell->kind != C_GLOBAL && cell->kind != C_LOCAL )
		report_error("Assignment not to pointer to memory but %s", cell_kind_name(cell));
	memory_p memory = cell->memory;
	int offset = cell->int_value;
	if (offset < 0)
		report_error("offset %d is negative", offset);
	if (offset >= memory->nr_cells * 4)
		report_error("offset %d outside of memory (%d)", offset, memory->nr_cells);
	if (is_word && (offset & 3) != 0)
		report_error("offset %d, not multiple of 4", offset);
	return   cell->kind == C_GLOBAL
		   ? &cell->memory->cells[offset / 4]
		   : &locals_stack[cell->locals_offset + offset / 4];
}

bool ignore_undefined = TRUE;
bool no_undefined_warnings = TRUE;

int get_array_byte(cell_p cell, int index)
{
	if (cell->kind != C_GLOBAL && cell->kind != C_LOCAL && cell->kind != C_STRING)
		report_error("get array byte not to pointer to memory but %s", cell_kind_name(cell));
	memory_p memory = cell->memory;
	int offset = cell->int_value + index;
	if (offset < 0)
		report_error("get array byte: offset %d is negative", offset);
	if (cell->kind == C_STRING)
	{
		if (offset > cell->str->length)
			report_error("get string byte: offset %d outside of string (%d, '%s')", offset, cell->str->length, cell->str->value);
		return cell->str->value[offset];
	}
	if (offset >= memory->nr_cells * 4)
		report_error("get array byte: offset %d outside of memory (%d)", offset, memory->nr_cells);
	cell_p elem =   cell->kind == C_GLOBAL
				  ? &cell->memory->cells[offset / 4]
				  : &locals_stack[cell->locals_offset + offset / 4];
	//if (cell->kind == C_LOCAL)
	//	printf("Contenst of local %d + %d = %d\n", cell->locals_offset, offset, cell->locals_offset + offset / 4);
	if (ignore_undefined && elem->kind == C_UNDEFINED)
	{
		if (!no_undefined_warnings)
			report_warning("Pointer is not pointing to array of bytes but undefined");
		return 0;

	}
	if (elem->kind != C_VALUE)
	{
		if (elem->kind == C_LOCAL)
			printf("%s %d.%d\n", elem->memory->name, elem->memory->line, elem->memory->column);
		report_error("Pointer is not pointing to array of bytes but %s", cell_kind_name(elem));
	}
	return 0xff & (elem->int_value >> (8 * (offset % 4)));
}

void set_array_byte(cell_p cell, int index, char ch)
{
	if (cell->kind != C_GLOBAL && cell->kind != C_LOCAL)
		report_error("set array byte not to pointer to memory");
	memory_p memory = cell->memory;
	int offset = cell->int_value + index;
	if (offset < 0)
		report_error("set array byte: offset %d is negative", offset);
	if (offset >= memory->nr_cells * 4)
		report_error("set array byte: offset %d outside of memory (%d)", offset, memory->nr_cells);
	cell_p elem =  cell->kind == C_GLOBAL
				 ? &cell->memory->cells[offset / 4]
				 : &locals_stack[cell->locals_offset + offset / 4];
	int byte_shift = 8 * (offset % 4);
	elem->kind = C_VALUE;
	elem->int_value = (elem->int_value & ~(0xFF << byte_shift)) | ((ch & 0xFF) << byte_shift);
	elem->command = cur_command;
}

bool opt_trace_assignments = FALSE;

void copy_cell(cell_p dst, cell_p src, bool set_command)
{
	memcpy(dst, src, sizeof(struct cell_s));
	if (set_command)
	{
		if (opt_trace_assignments)
			printf("Assign value from %d.%d to %d.%d\n",
				src->command->line, src->command->column,
				cur_command->line, cur_command->column);
		dst->command = cur_command;
	}
}

void sys_int80(void)
{
	check_stack(4);
	cell_p arg1 = &value_stack[value_depth - 3];
	if (arg1->kind != C_VALUE)
		report_error("First argument for int80 should be int, but it is %s", cell_kind_name(arg1));

	int result = 0;
	switch (arg1->int_value)
	{
		case 1:
			fprintf(fout, "EXIT\n");
			exit(1);
			break;
		case 3: // read loop: a1, buffer, 1
		{
			int a3 = pop_value();
			if (value_stack[value_depth-1].kind != C_VALUE)
				report_error("Second argument read should be int, but it is %s", cell_kind_name(&value_stack[value_depth-1]));
			int a1 = value_stack[value_depth-1].int_value;
			int bytes_read = 0;
			for (int i = 0; i < a3; i++)
			{
				char buf;
				if (read(a1, &buf, 1) == 0)
					break;
				set_array_byte(top_value, i, buf);
				bytes_read++;
			}
			pop();
			pop();
			result = bytes_read;
			break;
		}
		case 4: // write loop: a1, buffer, 1
		{
			int a3 = pop_value();
			if (value_stack[value_depth-1].kind != C_VALUE)
				report_error("Second argument write should be int, but it is %s", cell_kind_name(&value_stack[value_depth-1]));
			int a1 = value_stack[value_depth-1].int_value;
			int bytes_read = 0;
			for (int i = 0; i < a3; i++)
			{
				char buf = get_array_byte(top_value, i);
				if (write(a1, &buf, 1) == 0)
					break;
				bytes_read++;
			}
			pop();
			pop();
			result = bytes_read;
			break;
		}
		case 5: // open path, a2, a3
		{
			int a3 = pop_value();
			int a2 = pop_value();
			static char filename[300];
			for (int i = 0; i < 299; i++)
			{
				char ch = get_array_byte(top_value, i);
				filename[i] = ch;
				if (ch == '\0')
					break;
			}
			pop();
			result = open(filename, a2, a3);
			break;
		}
		case 6: // close a1
		{
			pop();
			pop();
			int a1 = pop_value();
			result = close(a1);
			break;
		}
		case 19: // lseek a1, a2, a3
		{
			int a3 = pop_value();
			int a2 = pop_value();
			int a1 = pop_value();
			result = lseek(a1, a2, a3);
			break; 
		}
		default:
			report_error("Unsupported int80 value %d", arg1->int_value);
	}
	pop();
	push_value(result);
}

cell_kind_e undefined_kind = C_UNDEFINED;

memory_p alloc_memory(int size)
{
	memory_p result = (memory_p)malloc(sizeof(struct memory_s));
	result->nr_cells = (size + 3) / 4;
	//printf("alloc_memory %d cells = %d", size, result->nr_cells);
	result->cells = (cell_p)malloc(result->nr_cells * sizeof(struct cell_s));
	result->name = "**heap**";
	result->line = cur_command != 0 ? cur_command->line : 0;
	result->column = cur_command != 0 ? cur_command->column : 0;
	for (int i = 0; i < result->nr_cells; i++)
	{
		result->cells[i].kind = undefined_kind;
		result->cells[i].int_value = 0;
		result->cells[i].command = NULL;
	}
	return result;
}

void sys_malloc(void)
{
	check_stack(1);
	if (top_value->kind != C_VALUE)
		report_error("Calling malloc with %s", cell_kind_name(top_value));
	//printf("sya_malloc size = %d\n", top_value->int_value);
	top_value->memory = alloc_memory(top_value->int_value);
	top_value->kind = C_GLOBAL;
	top_value->int_value = 0;
	top_value->command = cur_command;
}

void sys_realloc(void)
{
	check_stack(2);
	if (top_value->kind != C_VALUE)
		report_error("Calling realloc with %s for size", cell_kind_name(top_value));
	int size = pop_value();
	if (top_value->kind != C_VALUE && top_value->kind != C_GLOBAL)
		report_error("Calling realloc with %s for ptr", cell_kind_name(top_value));
	if (top_value->kind == C_VALUE && top_value->int_value != 0)
		report_error("Calling realloc with %d for ptr", top_value->int_value);
	//printf("sya_malloc size = %d\n", top_value->int_value);
	memory_p old_memory = top_value->kind == C_GLOBAL ? top_value->memory : NULL; 
	top_value->memory = alloc_memory(size);
	top_value->kind = C_GLOBAL;
	top_value->int_value = 0;
	top_value->command = cur_command;
	if (old_memory != NULL) {
		for (int i = 0; i < old_memory->nr_cells; i++)
			copy_cell(&top_value->memory->cells[i], &old_memory->cells[i], FALSE);
	}
}

int main(int argc, char *argv[])
{
	ferr = stderr;
	fout = stdout;
	fin = stdin;
	
	bool opt_trace_command = FALSE;
	bool opt_trace_functions = FALSE;
	int opt_indent_calls = 60000; //1307;

	const char **col_argv = (const char**)malloc(argc * sizeof(char*));
	int col_argc = 0;
	
	for (int i = 1; i < argc; i++)
	{
		const char *arg = argv[i];
		if (strcmp(arg, "-p") == 0)
			opt_trace_parsing = TRUE;
		else if (strcmp(arg, "-l") == 0)
			opt_trace_labels = TRUE;
		else if (strcmp(arg, "-d") == 0)
		{
			opt_trace_command = TRUE;
			opt_trace_assignments = TRUE;
			opt_trace_functions = TRUE;
		}
		else if (strcmp(arg, "-u") == 0)
			undefined_kind = C_VALUE;
		else
			col_argv[col_argc++] = arg;
	}

	if (col_argc == 0)
	{
		fprintf(ferr, "ERROR: No file specified\n");
		return 1;
	}
	fin = fopen(col_argv[0], "r");
	if (fin == 0)
	{
		fprintf(ferr, "ERROR: Cannot open file '%s' for input\n", col_argv[0]);
		return 1;
	}

	// Add predefined system functions
	ident_p sys_int80_ident = add_function("sys_int80");
	sys_int80_ident->function->sys_function = sys_int80;
	ident_p sys_malloc_ident = add_function("sys_malloc");
	sys_malloc_ident->function->sys_function = sys_malloc;
	ident_p sys_realloc_ident = add_function("sys_realloc");
	sys_realloc_ident->function->sys_function = sys_realloc;
	
	read_char();

	get_token();
	while (TRUE)
	{
		if (sym == '\0')
			break;
		
		if (opt_trace_parsing) printf("Parse %c\n", sym);
		if (sym == 'F')
		{
			// Function definition
			get_token();
			if (sym != 'A')
			{
				fprintf(ferr, "ERROR %d.%d: Expecting name after 'void' for function\n", cur_line, cur_column);
				return -1;
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
						ref_command = &idents[i].function->commands;
						break;
					}
				if (!found)
				{
					if (nr_idents >= MAX_NR_VARIABLES)
					{
						fprintf(ferr, "ERROR %d.%d: More than %d variables\n", cur_line, cur_column, MAX_NR_VARIABLES);
						return -1;
					}
					ident_p func = add_function(token);
					cur_function = func->function;
					if (opt_trace_parsing) printf("Entering function %s\n", cur_function->ident->name);
					ref_command = &cur_function->commands;					
				}
			}
			get_token();
			if (sym == ';')
			{
				// Forward definition
			}
			else
			{
				pos = 0;
				if (sym != '{')
				{
					fprintf(ferr, "ERROR %d.%d: Expect ; or { after function name\n", cur_line, cur_column);
					return -1;
				}
				func_labels = NULL;
				nesting_type[nesting_depth] = 'F';
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
			if (sym != 'A')
			{
				fprintf(ferr, "ERROR %d.%d: Expecting name after 'const'\n", cur_line, cur_column);
				return 1;
			}
			if (nr_idents >= MAX_NR_VARIABLES)
			{
				fprintf(ferr, "ERROR %d.%d: More than %d variables\n", cur_line, cur_column, MAX_NR_VARIABLES);
				return 1;
			}
			idents[nr_idents].type = 'C';
			strcpy(idents[nr_idents].name, token);
			get_token();
			if (sym != '0')
			{
				fprintf(ferr, "ERROR %d.%d: Expecting number after 'const' <name>\n", cur_line, cur_column);
				return 1;
			}
			idents[nr_idents].value = int_value;
			nr_idents++;
		}
		else if (sym == 'V' || sym == 'S')
		{
			char type = (sym == 'S' || nesting_depth == 0) ? 'M' : 'V'; 
			// Variable definition
			if (opt_trace_parsing) printf("Variable declaration %c %c\n", sym, type);
			get_token();
			int size = 1;
			if (sym == '0')
			{
				size = int_value;
				get_token();
			}
			if (sym != 'A')
			{
				fprintf(ferr, "ERROR %d.%d: Expecting name after 'int'\n", cur_line, cur_column);
				return 1;
			}
			if (nr_idents >= MAX_NR_VARIABLES)
			{
				fprintf(ferr, "ERROR %d.%d: More than %d variables\n", cur_line, cur_column, MAX_NR_VARIABLES);
				return 1;
			}
			bool repeated_global = FALSE;
			if (type == 'M')
			{
				for (int i = 0; i < nr_idents; i++)
					if (idents[i].type == 'M' && strcmp(idents[i].name, token) == 0)
					{
						repeated_global = TRUE;
						break;
					}
			}
			if (!repeated_global)
			{
				ident_p ident = &idents[nr_idents++];
				ident->type = type;
				strcpy(ident->name, token);
				ident->size = size;
				memory_p memory = (memory_p)malloc(sizeof(struct memory_s));
				ident->memory = memory;
				memory->nr_cells = size;
				memory->name = copystr(token);
				memory->line = cur_line;
				memory->column = cur_column;
				if (type == 'M')
				{
					memory->cells = (cell_p)malloc(size * sizeof(struct cell_s));
					for (int i = 0; i < size; i++)
					{
						memory->cells[i].kind = C_VALUE;
						memory->cells[i].int_value = 0;
						memory->cells[i].memory = NULL; // not applicable
					}
					ident->pos = 0;
				}
				else
				{
					memory->cells = NULL;
					ident->pos = pos;
					pos += size;
					if (pos > cur_function->max_locals_depth)
						cur_function->max_locals_depth = pos;
				}
			}
		}
		else if (sym == 'L')
		{
			get_token();
			if (sym != '{')
			{
				fprintf(ferr, "ERROR %d.%d: expecting '{' after 'do'\n", cur_line, cur_column);
				return 1;
			}
			add_label(find_label("_%s_loop%d", function_name, id));
			if (nesting_depth >= MAX_NESTING)
			{
				fprintf(ferr, "ERROR %d.%d: Nesting deeper than %d\n", cur_line, cur_column, MAX_NESTING);
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
				fprintf(ferr, "ERROR %d.%d: 'break' outside 'loop'\n", cur_line, cur_column);
				return 1;
			}
			if (sym == 'B')
				add_jump_command('J', find_label("_%s_loop_end%d", function_name, nesting_id[i]));
			else
				add_jump_command('J', find_label("_%s_loop%d", function_name, nesting_id[i]));
		}
		else if (sym == 'I')
		{
			get_token();
			if (sym != '{')
			{
				fprintf(ferr, "ERROR %d.%d: expecting '{' after 'if'\n", cur_line, cur_column);
				return 1;
			}
			add_jump_command('I', find_label("_%s_else%d", function_name, id));
			if (nesting_depth >= MAX_NESTING)
			{
				fprintf(ferr, "ERROR %d.%d: Nesting deeper than %d\n", cur_line, cur_column, MAX_NESTING);
				return 1;
			}
			nesting_type[nesting_depth] = 'I';
			nesting_id[nesting_depth] = id++;
			nesting_nr_vars[nesting_depth] = nr_idents;
			nesting_pos[nesting_depth] = pos;
			nesting_depth++;
		}
		else if (sym == 'E')
		{
			fprintf(ferr, "ERROR %d.%d: unexpected 'else'\n", cur_line, cur_column);
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
				fprintf(ferr, "ERROR %d.%d: To many }\n", cur_line, cur_column);
				return 1;
			}
			nesting_depth--;
			nr_idents = nesting_nr_vars[nesting_depth];
			pos = nesting_pos[nesting_depth];
			if (nesting_type[nesting_depth] == 'L')
			{
				add_jump_command('J', find_label("_%s_loop%d", function_name, nesting_id[nesting_depth]));
				add_label(find_label("_%s_loop_end%d", function_name, nesting_id[nesting_depth]));
			}
			else if (nesting_type[nesting_depth] == 'I')
			{
				get_token();
				if (sym == 'E')
				{
					get_token();
					if (sym != '{')
					{
						fprintf(ferr, "ERROR %d.%d: expecting '{' after 'else'\n", cur_line, cur_column);
						return 1;
					}
					add_jump_command('J', find_label("_%s_else_end%d", function_name, nesting_id[nesting_depth]));
					add_label(find_label("_%s_else%d", function_name, nesting_id[nesting_depth]));
					if (nesting_depth >= MAX_NESTING)
					{
						fprintf(ferr, "ERROR %d.%d: Nesting deeper than %d\n", cur_line, cur_column, MAX_NESTING);
						return 1;
					}
					nesting_type[nesting_depth] = 'E';
					nesting_nr_vars[nesting_depth] = nr_idents;
					nesting_pos[nesting_depth] = pos;
					nesting_depth++;
				}
				else
				{
					add_label(find_label("_%s_else%d", function_name, nesting_id[nesting_depth]));
					continue; // to skip the call to get_token at the end of the while loop
				}				
			}
			else if (nesting_type[nesting_depth] == 'E')
				add_label(find_label("_%s_else_end%d", function_name, nesting_id[nesting_depth]));
			else if (nesting_type[nesting_depth] == 'A')
				add_label(find_label("_%s_and_end%d", function_name, nesting_id[nesting_depth]));
			else if (nesting_type[nesting_depth] == 'O')
				add_label(find_label("_%s_or_end%d", function_name, nesting_id[nesting_depth]));
			else if (nesting_type[nesting_depth] == 'F')
			{
				if (opt_trace_parsing) printf("End function\n");
				for (label_p label = func_labels; label != NULL; label = label->prev)
					if (!label->defined)
						report_error("Label %s is never defined\n", label->name);
			}
		}
		else if (sym == 'G')
		{
			get_token();
			if (sym != 'A')
			{
				fprintf(ferr, "ERROR %d.%d: Expecting label after 'goto'\n", cur_line, cur_column);
				return 0;
			}
			add_jump_command('J', find_label("%s_%s", token, function_name));
		}
		else if (sym == 'A')
		{
			int i = nr_idents - 1;
			for (; i >= 0; i--)
				if (strcmp(token, idents[i].name) == 0)
					break;
			if (i < 0)
			{
				fprintf(ferr, "ERROR %d.%d: Ident %s is not defined\n", cur_line, cur_column, token);
				return -1;
			}
			//fprintf(fout, "# Ident %s type %c", token, idents[i].type);
			//if (idents[i].type == 'L')
			//	fprintf(fout, "\tpos %d", idents[i].pos);
			//else if (idents[i].type == 'C')
			//	fprintf(fout, "\tvalue %d", idents[i].value);
			//fprintf(fout, "\n");
			command_p command = add_command(idents[i].type);
			if (idents[i].type == 'F')
				command->function = idents[i].function;
			else if (idents[i].type == 'C')
				command->int_value = idents[i].value;
			else
			{
				command->memory = idents[i].memory;
				if (idents[i].type == 'V')
					command->int_value = idents[i].pos;
			}
		}
		else if (sym == '0')
		{
			command_p command = add_command(sym);
			command->int_value = int_value;
		}
		else if (sym == '"')
		{
			command_p command = add_command(sym);
			command->int_value = 0;
			command->str = unique_string(token, token_len);
		}
		else if (sym == '\'')
		{
			command_p command = add_command(sym);
			command->int_value = token[0];
		}
		else if (sym == ':')
		{
			get_token();
			if (sym != 'A')
			{
				fprintf(ferr, "ERROR %d.%d: Expect identifier after ':", cur_line, cur_column);
				return -1;
			}
			add_label(find_label("%s_%s", token, function_name));
		}
		else if (sym == SYM_LOG_AND)
		{
			get_token();
			if (sym != '{')
			{
				fprintf(ferr, "ERROR %d.%d: expecting '{' after '&&'\n", cur_line, cur_column);
				return 1;
			}
			add_jump_command(SYM_LOG_AND, find_label("_%s_and_end%d", function_name, id));
			if (nesting_depth >= MAX_NESTING)
			{
				fprintf(ferr, "ERROR %d.%d: Nesting deeper than %d\n", cur_line, cur_column, MAX_NESTING);
				return 1;
			}
			nesting_type[nesting_depth] = 'A';
			nesting_id[nesting_depth] = id++;
			nesting_nr_vars[nesting_depth] = nr_idents;
			nesting_pos[nesting_depth] = pos;
			nesting_depth++;
		}
		else if (sym == SYM_LOG_OR)
		{
			get_token();
			if (sym != '{')
			{
				fprintf(ferr, "ERROR %d.%d: expecting '{' after '||'\n", cur_line, cur_column);
				return 1;
			}
			add_jump_command(SYM_LOG_OR, find_label("_%s_or_end%d", function_name, id));
			if (nesting_depth >= MAX_NESTING)
			{
				fprintf(ferr, "ERROR %d.%d: Nesting deeper than %d\n", cur_line, cur_column, MAX_NESTING);
				return 1;
			}
			nesting_type[nesting_depth] = 'O';
			nesting_id[nesting_depth] = id++;
			nesting_nr_vars[nesting_depth] = nr_idents;
			nesting_pos[nesting_depth] = pos;
			nesting_depth++;
		}
		else if (sym == SYM_ARROW)
		{
			get_token();
			if (sym != 'A')
			{
				fprintf(ferr, "ERROR %d.%d: Expecting const ident after '->'. Found %s\n", cur_line, cur_column, token);
				return 1;
			}
			int i = nr_idents - 1;
			for (; i >= 0; i--)
				if (strcmp(token, idents[i].name) == 0)
					break;
			if (i < 0 || idents[i].type != 'C')
			{
				fprintf(ferr, "ERROR %d: Ident %s is not defined\n", cur_line, token);
				error = 1;
			}
			else
			{
				command_p command = add_command(SYM_ARROW);
				command->int_value = idents[i].value;
			}
		}
		else if (sym == ' ')
		{
			fprintf(ferr, "ERROR %d.%d: token |%s| not supported\n", cur_line, cur_column, token);
			return -1;
		}
		else
		{
			command_p command = add_command(sym);
			if (sym == SYM_CALL)
				command->int_value = pos;
		}
		
		get_token();
	}


	cur_function = NULL;
	{
		int i = nr_idents - 1;
		for (; i >= 0; i--)
			if (strcmp("main", idents[i].name) == 0)
			{
				cur_function = idents[i].function;
				break;
			}
	}
	if (cur_function == NULL)
	{
		fprintf(ferr, "No 'main' function\n");
		return -1;
	}
	cur_command = cur_function->commands;
	if (cur_command == NULL)
	{
		fprintf(ferr, "Empty 'main'\n");
		return -1;
	}


	for (int i = 0; i < idents[i].function->max_locals_depth; i++)
		locals_stack[i].kind = C_UNDEFINED;

	top_value = &value_stack[value_depth];

	{
		int nr_env = 0;
		while (argv[argc + nr_env + 1] != NULL)
			nr_env++;

		// Push argv
		push_value(col_argc);
		push(C_GLOBAL);
		top_value->memory = alloc_memory(4 * (col_argc + 1 + nr_env + 1));
		for (int i = 0; i < col_argc; i++)
		{
			top_value->memory->cells[i].kind = C_STRING;
			top_value->memory->cells[i].str = unique_string(col_argv[i], strlen(col_argv[i]));
			top_value->memory->cells[i].command = cur_command;
		}
		top_value->memory->cells[col_argc].kind = C_VALUE;
		for (int i = 0; i < nr_env; i++)
		{
			int env_i = col_argc + 1 + i;
			top_value->memory->cells[env_i].kind = C_STRING;
			top_value->memory->cells[env_i].str = unique_string(argv[argc + 1 + i], strlen(argv[argc + 1 + i]));
			top_value->memory->cells[env_i].command = cur_command;
		}
		top_value->memory->cells[col_argc + 1 + nr_env].kind = C_VALUE;
	}

	int indent = 0;

	while (TRUE)
	{
		while (cur_command == NULL)
		{
			if (opt_trace_functions) printf("function depth = %d\n", function_depth);
			if (--function_depth < 0)
				return 0;
			if (opt_trace_functions) printf("Leaving function %s\n", cur_function->ident->name);
			cur_function = function_stack[function_depth].function;
			cur_command = function_stack[function_depth].command;
			if (opt_trace_functions) printf("Continue with %s at %d.%d\n", cur_function->ident->name,
				cur_command->line, cur_command->column);
			if (cur_command == NULL || cur_command->sym != SYM_CALL)
				report_error("Should be on call");
			if (opt_trace_functions) printf("Decrement locals_offset from %d with %d to %d\n", locals_offset, cur_command->int_value, locals_offset - cur_command->int_value);
			if (cur_command->line > opt_indent_calls)
			{
				indent--;
				for (int i = 0; i < indent; i++) printf("  ");
				printf("returned\n");
			}
			locals_offset -= cur_command->int_value;
			cur_command = cur_command->next;
		}

		if (opt_trace_command)
		{
			printf("Execute command %d.%d: %c %s (%d:", cur_command->line, cur_command->column, cur_command->sym, command_name(cur_command), value_depth);
			print_value_stack(stdout);
		}

		char sym = cur_command->sym;
		if (sym == 'I')
		{
			if (!pop_bool())
				cur_command = cur_command->jump_command;
			else
				cur_command = cur_command->next;
			continue;
		}
		if (sym == 'J')
		{
			cur_command = cur_command->jump_command;
			continue;
		}
		if (sym == 'R')
		{
			cur_command = NULL;
			continue;
		}
		if (sym == ';')
		{
			pop();
		}
		else if (sym == '$')
		{
			push(C_UNDEFINED);
			*top_value = value_stack[value_depth - 1];
			top_value->command = cur_command;
		}
		else if (sym == 'M')
		{
			push(C_GLOBAL);
			top_value->int_value = 0;
			top_value->memory = cur_command->memory;
		}
		else if (sym == 'V')
		{
			push(C_LOCAL);
			top_value->int_value = 0;
			top_value->locals_offset = locals_offset + cur_command->int_value;
			top_value->memory = cur_command->memory;
		}
		else if (sym == 'F')
		{
			push(C_FUNCTION);
			top_value->function = cur_command->function;
		}
		else if (sym == 'K')
		{
			unsigned int value = get_top_value();
			top_value->int_value = (int)(char)value;
		}
		else if (sym == '0' || sym == 'C' || sym == '\'')
		{
			push_value(cur_command->int_value);
		}
		else if (sym == '"')
		{
			push(C_STRING);
			top_value->int_value = 0;
			top_value->str = cur_command->str;
		}
		else if (sym == '?')
		{
			check_stack(1);
			copy_cell(top_value, deref(top_value, TRUE), FALSE);
			if (ignore_undefined && top_value->kind == C_UNDEFINED)
			{
				if (!no_undefined_warnings)
					report_warning("Retrieved undefined value");
				top_value->kind = C_VALUE;
				top_value->int_value = 0;
			}
		}
		else if (sym == '=')
		{
			check_stack(2);
			cell_p lhs = &value_stack[value_depth-1];
			copy_cell(deref(lhs, TRUE), top_value, TRUE);
			copy_cell(lhs, top_value, FALSE);
			pop();
		}
		else if (sym == '+')
		{
			check_stack(2);
			int rhs_value = pop_value();
			if (top_value->kind != C_VALUE && top_value->kind != C_GLOBAL && top_value->kind != C_LOCAL && top_value->kind != C_STRING)
				report_error("Cannot add with '%s'", cell_kind_name(top_value));
			top_value->int_value += rhs_value;
			top_value->command = cur_command;
		}
		else if (sym == '-' || sym == SYM_SUB_PTRS)
		{
			check_stack(2);
			cell_p lhs = &value_stack[value_depth-1];
			if (top_value->kind == C_VALUE && (lhs->kind == C_VALUE || lhs->kind == C_LOCAL || lhs->kind == C_GLOBAL || lhs->kind == C_STRING))
			{
				if (sym == SYM_SUB_PTRS && lhs->kind != C_VALUE)
				{
					if (top_value->int_value != 0)
						report_error("Substract non-zero value with pointer not allowed");
					int value = (long)lhs->memory + lhs->int_value;
					pop();
					top_value->kind = C_VALUE;
					top_value->int_value = value;
				}
				else
				{
					int rhs_value = pop_value();
					top_value->int_value -= rhs_value;
				}
			}
			else if (   (lhs->kind == C_LOCAL && top_value->kind == C_LOCAL)
					 || (lhs->kind == C_GLOBAL && top_value->kind == C_GLOBAL))
			{
				if (lhs->memory != top_value->memory)
					report_error("Substraction between incorrect pointers");
				int sub = lhs->int_value - top_value->int_value;
				pop();
				top_value->kind = C_VALUE;
				top_value->int_value = sub;
			}
			else if (lhs->kind == C_STRING && top_value->kind == C_STRING)
			{
				if (lhs->str != top_value->str)
					report_error("Substraction between incorrect strings");
				int sub = lhs->int_value - top_value->int_value;
				pop();
				top_value->kind = C_VALUE;
				top_value->int_value = sub;
			}
			else if (lhs->kind == C_VALUE && lhs->int_value == 0 && top_value->kind == C_GLOBAL)
			{
				int sub = -((long)top_value->memory + top_value->int_value);
				pop();
				top_value->kind = C_VALUE;
				top_value->int_value = sub;
			}
			else
				report_error("Illegal substraction (%d:%d - %d)", lhs->kind, lhs->int_value, top_value->kind);
			top_value->command = cur_command;
		}
		else if (sym == SYM_REV_ASS)
		{
			check_stack(2);
			cell_p lhs = &value_stack[value_depth-1];
			copy_cell(deref(top_value, TRUE), lhs, TRUE);
			pop();
			pop();
		}
		else if (sym == SYM_GET_BYTE)
		{
			check_stack(1);
			int value = get_array_byte(top_value, 0);
			top_value->kind = C_VALUE;
			top_value->int_value = value;
			top_value->command = cur_command;
		}
		else if (sym == SYM_ASS_BYTE)
		{
			check_stack(2);
			int value = pop_value() & 0xff;
			set_array_byte(top_value, 0, value);
			top_value->kind = C_VALUE;
			top_value->int_value = value;
		}
		else if (sym == SYM_CALL)
		{
			check_stack(1);
			if (top_value->kind != C_FUNCTION)
				report_error("Can only call a function");
			function_p function = top_value->function;
			pop();
			if (function->sys_function != 0)
				function->sys_function();
			else
			{
				if (opt_trace_functions) printf("Increment locals_offset from %d with %d to %d\n", locals_offset, cur_command->int_value, locals_offset + cur_command->int_value);
				if (cur_command->line > opt_indent_calls)
				{
					for (int i = 0; i < indent; i++) printf("  ");
					printf("Call at %d to %d\n", cur_command->line, function->commands->line);
					indent++;
				}
				locals_offset += cur_command->int_value;
				if (locals_offset + cur_function->max_locals_depth >= MAX_LOCALS_DEPTH)
					report_error("Stack locals overflow");
				for (int i = 0; i < cur_function->max_locals_depth; i++)
					locals_stack[locals_offset + i].kind = C_UNDEFINED;
				function_stack[function_depth].function = cur_function;
				function_stack[function_depth].command = cur_command;
				if (++function_depth >= MAX_FUNCTION_DEPTH)
					report_error("Function stack overflow");
				cur_function = function;
				cur_command = function->commands;
				continue;
			}
		}
		else if (sym == SYM_ARROW)
		{
			check_stack(1);
			copy_cell(top_value, deref(top_value, TRUE), TRUE);
			if (top_value->kind != C_GLOBAL && top_value->kind != C_LOCAL)
				report_error("Arrow needs pointer");
			top_value->int_value += cur_command->int_value;
		}
		else if (sym == SYM_SWAP)
		{
			check_stack(2);
			cell_t cell;
			copy_cell(&cell, top_value, FALSE);
			copy_cell(top_value, &value_stack[value_depth-1], FALSE);
			copy_cell(&value_stack[value_depth-1], &cell, FALSE);
		}
		else if (sym == '~')
		{
			unsigned int value = get_top_value();
			top_value->int_value = ~value;
		}
		else if (sym == '!')
		{
			bool value = get_top_bool();
			top_value->kind = C_VALUE;
			top_value->int_value = !value;
		}
		else if (sym == SYM_LOG_AND)
		{
			bool value = get_top_bool();
			if (!value)
			{
				cur_command = cur_command->jump_command;
				continue;
			}
			pop();
		}
		else if (sym == SYM_LOG_OR)
		{
			bool value = get_top_bool();
			if (value)
			{
				cur_command = cur_command->jump_command;
				continue;
			}
			pop();
		}
		else if (sym == SYM_EQ || sym == SYM_NE)
		{
			check_stack(2);
			cell_p lhs = &value_stack[value_depth-1];
			bool equal = FALSE;
			if (lhs == top_value)
				equal = TRUE;
			else if (   lhs->kind != top_value->kind 
					 && !(   (lhs->kind == C_GLOBAL && top_value->kind == C_LOCAL)
						  || (lhs->kind == C_LOCAL && top_value->kind == C_GLOBAL)))
			{
				if (   (top_value->kind == C_VALUE && top_value->int_value == 0 && lhs->kind != C_UNDEFINED)
					|| (lhs->kind == C_VALUE && lhs->int_value == 0 && top_value->kind != C_UNDEFINED))
					; // Accept comparing 'pointer' with NULL
				else if (top_value->kind == C_VALUE && lhs->kind == C_GLOBAL)
					report_warning("== != comparing value with pointer");
				else
					report_warning("== != cannot compare different types");
			}
			else if (top_value->kind == C_UNDEFINED)
				report_warning("== != cannot compare undefined values");
			else
				equal =    lhs->int_value == top_value->int_value
						&& (  lhs->kind == C_VALUE ? TRUE
							: lhs->kind == C_GLOBAL ? lhs->memory == top_value->memory
							: lhs->kind == C_LOCAL ? lhs->memory == top_value->memory
							: lhs->kind == C_STRING ? lhs->str == top_value->str
							: lhs->kind == C_FUNCTION && lhs->function == top_value->function);
			pop();
			pop();
			push_value(equal == (sym == SYM_EQ));
		}
		else if (sym == '<' || sym == '>' || sym == SYM_LE || sym == SYM_GE)
		{
			check_stack(2);
			cell_p lhs = &value_stack[value_depth-1];
			if (lhs->kind != top_value->kind)
			{
				if (lhs->kind == C_GLOBAL && top_value->kind == C_VALUE && top_value->int_value == 0)
				{
					pop();
					pop();
					push_value(sym == '>');
				}
				else
					report_error("< <= > >= cannot compare different types");
			}
			else
			{
				if (top_value->kind == C_UNDEFINED)
					report_error("< <= > >= cannot compare undefined values");
				if (top_value->kind == C_FUNCTION)
					report_error("< <= > >= cannot compare function pointers");
				int diff = 0;
				if (top_value->kind == C_GLOBAL || top_value->kind == C_LOCAL)
				{
					if (lhs->memory != top_value->memory)
					{
						report_warning("< <= > >= compare pointers from different memory pieces");
						diff = top_value->memory - lhs->memory;
					}
				}
				else if (top_value->kind == C_STRING)
				{
					if (lhs->str != top_value->str)
					{
						report_warning("< <= > >= compare pointers from different string");
						diff = top_value->str - lhs->str;
					}
				}
				unsigned int urhs = top_value->int_value;
				pop();
				unsigned int ulhs = top_value->int_value;
				pop();
				if (diff > 0)
					urhs += diff;
				else if (diff < 0)
					ulhs -= diff;
				if (sym == '<')
					push_value(ulhs < urhs);
				else if (sym == '>')
					push_value(ulhs > urhs);
				else if (sym == SYM_LE)
					push_value(ulhs <= urhs);
				else // (sym == SYM_GE)
					push_value(ulhs >= urhs);
			}
		}
		else
		{
			int rhs = pop_value();
			unsigned int urhs = rhs;
			int lhs = pop_value();
			unsigned int ulhs = lhs;
			if (sym == '*')
				push_value(lhs * rhs);
			else if (sym == '&')
				push_value(ulhs & urhs);
			else if (sym == '|')
				push_value(ulhs | urhs);
			else if (sym == '^')
				push_value(ulhs ^ urhs);
			else if (sym == '/')
				push_value(ulhs / urhs);
			else if (sym == '%')
				push_value(ulhs % urhs);
			else if (sym == SYM_DIV_SIGNED)
				push_value(lhs / rhs);
			else if (sym == SYM_MOD_SIGNED)
				push_value(lhs % rhs);
			else if (sym == SYM_LT_SIGNED)
				push_value(lhs < rhs);
			else if (sym == SYM_LE_SIGNED)
				push_value(lhs <= rhs);
			else if (sym == SYM_GT_SIGNED)
				push_value(lhs > rhs);
			else if (sym == SYM_GE_SIGNED)
				push_value(lhs >= rhs);
			else if (sym == SYM_SHL)
				push_value(ulhs << urhs);
			else if (sym == SYM_SHR)
				push_value(ulhs >> urhs);
			else
				report_error("Unknown command %d", sym);
		}
		cur_command = cur_command->next;
	}


	return error;
}