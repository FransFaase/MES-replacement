# The C-compiler to compile the Tiny C Compiler

This C-compiler with build-in C-preprocessor presumes that the
input is correct. It only reports warning or errors when an
internal limit is being exceeded, such as the maximum length of
a token, or when parsing cannot proceed.

A list of limitations:
- No support for floats.

It generates code for the [Stack-C](Stack_C.md) intermediate
language.

## Preprocessor

The compiler includes a preprocessor that implements the necessary
parts of the C-preprocessor to compile the Tiny C Compiler sources
and other needed sources for standard library and tools needed to
implement stage0.

The preprocessor is implemented as a series of character and token
iterators (partly also to demonstrate that this is possible). All
character iterators use the struct `char_iterator_t` as it base
and all token iterator use the struct `token_iterator_t` as it
base. The following iterators are implemented

### The file iterator

The struct `file_iterator_s` and the function `file_iterator_next`
implement the file iterator.
On creation it points to the first character of the file
and on each call to the next function it returns the next
character of the file, where the file is thought to be
extended with an infinite number of null ('\0') characters
and carriage return characters ('\r') are skipped.
The line and column members are updated according to
line feed ('\n') characters in the file.

### The line splice iterator

The struct `line_splice_iterator_s` and the function
`line_splice_iterator_next` implement the line spice iterator,
which joins lines from the input when the black-slach (`\\`)
character appears as the last character on the line.
On creation the iterator points to the first
character in the file, assuming that pairs of a
back-slash charater and line-feed characters are ignored.
On each call of the next function, the following character
ignoring such pairs is returned.

### The comment strip iterator

The struct `comment_strip_iterator_s` and the function
`comment_strip_iterator_next` implement the comment strip
iterator, which removes all comments from the input.
It supports both the original comments which are delimited by
'/*' and '*/' and the new comments with are delimited by '//'
and a new-line character. The original comments are replaced
by a space character and the new comments are replace by a
new-line character.
The function `comment_strip_iterator_next` is implemented as a
co-routine with the help of goto statements.

### The include iterator

The struct `include_iterator_s` and the function
`include_iterator_next` implement the include iterator, which
allows, by calling the function `include_iterator_add`, to
include the stream of character returned by another character
iterator to be included. Recursive including is supported.

### The tokenizer iterator

The struct `tokenizer_s` and the function `tokenizer_next`
implement the tokenizer iterator that recognizes the C tokens
from the stream of characters returned by a character iterator.
On creation it is on the first recognized token and
on each call to the next function it returns the next
recognized token.

The tokenizer assumees that all integer values are within
the range of `int`.

### The conditional iterator

The struct `conditional_iterator_s` and the function
`conditional_iterator_next` implement the conditional
iterator that deals with all conditional compilation
directives, such as `#if`, and also the directives
`#define`, `#undef`, `#include`, and `#error`.

It does not support macro expansion in conditional expressions.

It implements the following grammar for the conditions,
where `conditional_or_expr` is the root:
```
	conditional_primary
		: '(' conditional_or_exp ')'
		| 'defined' ( '(' ident ')' |  ident )
		| integer
		| ident
		.
	conditional_unary_expr
		: '!' conditional_primary
		| conditional_primary
		.
	conditional_compare_expr
		: conditional_unary_expr
		  ('==' conditional_unary_expr 
		  |'!=' conditional_unary_expt ) OPT
		.
	conditional_and_expr : conditional_compare_expr CHAIN '&&' .
	conditional_or_expr : conditional_and_expr CHAIN '||' .
```

### The expand macro iterator

The struct `expand_macro_iterator` and the function
`expand_macro_iterator_next` implement the expansion
of a macro with a list of tokens for the arguments.
It also implements processing of the `#` and `##` operators
during expansion. After expansion has been completed,
it returns the token iterator for the remaining part.

### The expand iterator

The struct `expand_iterator_s` and the function
`expand_iterator_next` implement the expansion of all
defined symbols and macros. For the expansion of these
the expand macro iterator is used.

## The compiler

The compiler is a one pass compiler that generates code
for the statements on the fly. Expressions are parsed
in an abstract syntax tree, before code is generated
for them. (It was decided to group the initialization
of all global variables together, which makes it
necessary to store the expressions for these. As an
alternative a separate function for initializing global
variables could have been used.)

The C-compiler parses the input according to the
following grammar, where `program` is the root:
```
	primary_expr 
		: identifier
		| integer
		| char
		| string
		| '(' expr ')'
		.
	postfix_expr
		: primary_expr
		| postfix_expr '[' expr ']'
		| postfix_expr '(' assignment_expr LIST ')'
		| postfix_expr '.' ident
		| postfix_expr '->' ident
		| postfix_expr '++'
		| postfix_expr '--'
		.
	unary_expr
		: '++' unary_expr
		| '--' unary_expr
		| '&' cast_expr
		| '*' cast_expr
		| '+' cast_expr
		| '-' cast_expr
		| '~' cast_expr
		| '!' cast_expr
		| 'sizeof' '(' sizeof_type ')'
		| 'sizeof' unary_expr
		| postfix_expr
		.
	sizeof_type
		: 'char'
		| 'int'
		| 'unsigned' ('int') OPT
		| 'double'
		| 'void' ('*') OPT
		| 'struct' ident
		| ident
		| sizeof_type '*'
		.
	cast_expr
		: '(' abstract_declaration ')' cast_expr
		| unary_expr
		.
	l_expr1
		: cast_expr
		| l_expr1 '*' cast_expr
		| l_expr1 '/' cast_expr
		| l_expr1 '%' cast_expr
		.
	l_expr2
		: l_expr1
		| l_expr2 '+' l_expr1
		| l_expr2 '-' l_expr1
		.
	l_expr3
		: l_expr2
		| l_expr3 '<<' l_expr2
		| l_expr3 '>>' l_expr2
		.
	l_expr4
		: l_expr3
		| expr4 '<=' l_expr3
		| expr4 '>=' l_expr3
		| expr4 '<' l_expr3
		| expr4 '>' l_expr3
		| expr4 '==' l_expr3
		| expr4 '!=' l_expr3
		.
	l_expr5
		: l_expr4
		| l_expr5 '^' l_expr4
		.
	l_expr6
		: l_expr5
		| l_expr6 '&' l_expr5
		.
	l_expr7
		: l_expr6
		| l_expr7 '|' l_expr6
		.
	l_expr8
		: l_expr7
		| l_expr8 '&&' l_expr7
		.
	l_expr9
		: l_expr8
		| l_expr9 '||' l_expr8
		.
	conditional_expr
		: l_expr9 '?' l_expr9 ':' conditional_expr
		| l_expr9
		.
	assignment_expr
		: conditional_expr 
		  (( '=' | '*=' | '/=' | '%=' | '+=' | '-=' | '<<=' | '>>=' | '&=' | '|=' | '^=')
		   assignment_expr
		  ) SEQ
		.
	expr : assignment_expr LIST
	array_indexes : expr ']' ('[' array_indexes) OPT .
	declaration
		: ('typedef' | 'extern' | 'inline' | 'static' ) SEQ OPT
		  type_specifier
		  ( ';'
		  | func_declarator '(' declaration LIST (',' '...') OPT ')'
		    ( ';'
			| '{' statements '}'
			)
		  | ( declarator ( '=' initializer ) OPT ) LIST ";"
		  )
		.
	func_declarator
		: '*' OPT SEQ ( ident | '(' '*' ident ')')
		.
	declarator
		: '*' OPT SEQ
		  ( ident | '(' '*' ident ')')
		  ( ('[' ']') OPT '[' array_indexes ) OPT
		.
	type_specifier
		: 'const' OPT
		  ( 'char' 'const' OPT
		  | 'unsigned' ('char' | 'short' | 'long' 'long' OPT | 'int') OPT
		  | 'short'
		  | 'int'
		  | 'long' ('double' | 'long') OPT
		  | 'float'
		  | 'double'
		  | 'void'
		  | 'struct' struct_or_union_specifier
		  | 'union' struct_or_union_specifier
		  | 'enum' enum_specifier
		  | ident
		  )
		.
	struct_or_union_specifier : ident OPT ('{' declaration SEQ OPT "}") OPT .
	enum_specifier : ident OPT ('{' (ident ('=' constant_expr) OPT ) LIST '}') OPT .
	initializer
		: '{' initializer LIST OPT ',' OPT '}'
		| assignment_expr
		.
	statement
		: (ident ':') SEQ OPT
		  ( 'if' '(' expr ')' statement ('else' statement) OPT
		  | 'while' '(' expr ')' statement
		  | 'do' statement 'while' '(' expr ')' ';'
		  | 'for' '(' (declaration | expr ';') expr ';' expr ')' statement
		  | 'break' ';'
		  | 'continue' ';'
		  | 'switch' '(' expr ')' '{'
		  	 ( ( 'case' expr ':' | 'default' ':') SEQ statement SEQ ) SEQ '}'
		  | 'return' expr OPT ';'
		  | 'goto' ident ';'
		  | '{' statements '}'
		  | expr ';'
		  )
		.
	statements : (declaration | statement) SEQ OPT .
    program : declaration SEQ OPT .

```


