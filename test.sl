# standard functions

const stdin 0
const stdout 1
const stderr 2

func strcmp
{
	var b b =
	var a a =
	var result
    loop
    {
		a ? ?1 b ? ?1 - result = 
		result ? 0 != or { a ? ?1 0 == } then { break }
		# { break }
		a dup ? 1 + =:
		b dup ? 1 + =:
	}
	result ?
	ret
}

func fhputs
{
	var f f =
	var s s =
	loop
	{
		s ? ?1 0 == then { break }
		s ? ?1 f ? sys_fputc () pop
		s dup ? 1 + =:
	}
	ret
}

func fhput_int
{
	var f f =
	var i i =
	i ? 0 == then
	{
		'0' f ? sys_fputc() pop
		ret
	}
	i ? 0 <s then
	{
		'-' f ? sys_fputc() pop
		0 i ? - i =
	}
	0
	loop
	{
		i ? 0 == then { break }
		i ? 10 % '0' +
		i ? 10 / i =
	}
	loop
	{
		i =
		i ? 0 == then { break }
		i ? f ? sys_fputc() pop
	}
	ret
}

# Helper functions

func copystr
{
	var str str =

	var len 0 len =
	loop
	{
		str ? len ? + ?1 0 == then { break }
		len dup ? 1 + =:
	}
	len ? 1 + sys_malloc() var new_str new_str =
	
	loop
	{
		str ? len ? + ?1 new_str ? len ? + =1
		len ? 0 == then { break }
		len dup ? 1 - =:
	}
	new_str ?
	ret
}
			

# char_iterator

const chit_ch 0
const chit_line 4
const chit_column 8
const chit_filename 12
const chit_next 16

# file_iterator

const f_it_f 20

func file_iterator_next
{
	var it it =
	it->chit_ch ?1 0 == then { ret }
	it->chit_ch ?1 '\n' == then
	{
		it->chit_line dup ? 1 + =: 
		0 it->chit_column =
	}
	it->chit_column dup ? 1 + =:
	it->f_it_f ? sys_fgetc() it->chit_ch =
	it->chit_ch ?1 0 == then
	{
		it->f_it_f ? sys_close()
	}
	else
	{
		it ? chit_ch + ? '\r' == then
		{
			it ? file_iterator_next()
		}
	}
	ret
}

func new_file_iterator
{
	var fn fn =
	24 sys_malloc() var it it =
	fn ? 0 0 sys_open() it->f_it_f =
	'\n' it->chit_ch =
	it->f_it_f ? 0 < then
	{
		"new_file_iterator: Could not open file\n" stdout fhputs
		0 it->chit_ch =
	}
	fn ? copystr() it->chit_filename =
	0 it->chit_line =
	0 it->chit_column =
	file_iterator_next it->chit_next =
	it ? file_iterator_next()
	it ?
	ret
}

# line_splice_iterator

const ls_it_source_it 20
const ls_it_a 24

func line_splice_iterator_next
{
	var it it =
	
	it->ls_it_a ?1 0 == then
	{
		0 it->chit_ch =
		ret
	}
	
	it->ls_it_a ?1 ' ' == or { it->ls_it_a ?1 '\t' == } then
	{
		it->ls_it_source_it ? chit_line + ? it->chit_line =
		it->ls_it_source_it ? chit_column + ? it->chit_column =
		loop
		{
			it->ls_it_a ?1 ' ' != and { it->ls_it_a ?1 '\t' != } then { break }
			it->ls_it_source_it ? dup chit_next + ? ()
			it->ls_it_source_it ? chit_ch + ?1 it->ls_it_a =1
		}
		' ' it->chit_ch =
		ret
	}
	it->ls_it_a ?1 it->chit_ch =1
	it->ls_it_source_it->chit_filename ? it->chit_filename =
	it->ls_it_source_it->chit_line ? it->chit_line =
	it->ls_it_source_it->chit_column ? it->chit_column =
	it->ls_it_source_it ? dup chit_next + ? ()
	it->ls_it_source_it->chit_ch ?1 it->ls_it_a =1
	loop
	{
		it->chit_ch ?1 '\\' != or { it->ls_it_a ?1 '\n' != } then { break }
		it->ls_it_source_it ? dup chit_next + ? ()
		it->ls_it_source_it->chit_ch ?1 it->chit_ch =1
		it->ls_it_source_it->chit_filename ? it->chit_filename =
		it->ls_it_source_it->chit_line ? it->chit_line =
		it->ls_it_source_it->chit_column ? it->chit_column =
		it->ls_it_source_it ? dup chit_next + ? ()
		it->ls_it_source_it->chit_ch ?1 it->ls_it_a =1
	}
	ret
}

func new_line_splice_iterator
{
	var source_it source_it =
	
	28 sys_malloc () var it it =
	source_it ? it->ls_it_source_it =
	source_it->chit_filename ? it->chit_filename =
	line_splice_iterator_next it->chit_next =
	source_it->chit_ch ?1 it->ls_it_a =1
	it ? line_splice_iterator_next()
	it ?
	ret 
}

# comment_strip_iterator

const cs_it_source_it 20
const cs_it_a 24
const cs_it_state 28

func comment_strip_iterator_next
{
	var it it =
	
	it->cs_it_state ? 1 == then { goto s1 }
	it->cs_it_state ? 2 == then { goto s2 }
	it->cs_it_state ? 3 == then { goto s3 }
	it->cs_it_state ? 4 == then { goto s4 }
	it->cs_it_state ? 5 == then { goto s5 }
	it->cs_it_state ? 6 == then { goto s6 }
	it->cs_it_state ? 7 == then { goto s7 }
	it->cs_it_state ? 8 == then { goto s8 }
	it->cs_it_state ? 9 == then { goto s9 }
	
	:s0
	it->cs_it_a ? '\0' == then
	{
		'\0' it->chit_ch + =
		ret
	}
	it->cs_it_a ?1 it->chit_ch =1
	it->cs_it_source_it->chit_filename ? it->chit_filename =
	it->cs_it_source_it->chit_line ? it->chit_line =
	it->cs_it_source_it->chit_column ? it->chit_column =
	it->cs_it_source_it ? dup chit_next + ? ()
	it->cs_it_source_it->chit_ch ?1 it->cs_it_a =1
	it->chit_ch ?1 '/' == and { it->cs_it_a ?1 '/' == or { it->cs_it_a ?1 '*' == } } then
	{
		it->cs_it_a ?1 '/' == then
		{
			loop
			{
				it->cs_it_a ?1 '\0' == or { it->cs_it_a ?1 '\n' == } then { break }
				it->cs_it_source_it ? dup chit_next + ? ()
				it->cs_it_source_it->chit_ch ?1 it->cs_it_a =1
			}
		}
		else
		{
			' ' it->chit_ch =
			1 it->cs_it_state = ret :s1
			it->cs_it_source_it ? dup chit_next + ? ()
			it->cs_it_source_it->chit_ch ?1 it->chit_ch =1
			it->cs_it_source_it ? dup chit_next + ? ()
			it->cs_it_source_it->chit_ch ?1 it->cs_it_a =1
			loop
			{
				it->chit_ch ?1 '\0' == or { it->chit_ch ?1 '*' == and { it->cs_it_a ?1 '/' == } } then { break }
				it->cs_it_a ?1 it->chit_ch =1
				it->cs_it_source_it ? dup chit_next + ? ()
				it->cs_it_source_it->chit_ch ?1 it->cs_it_a =1
			}
			it->chit_ch ?1 0 != then
			{
				it->cs_it_source_it ? dup chit_next + ? ()
				it->cs_it_source_it->chit_ch ? it->cs_it_a =
				it->cs_it_source_it->chit_line ? it->chit_line =
				it->cs_it_source_it->chit_column ? it->chit_column =
			}
		}
		0 it->cs_it_state =
		goto s0
	}
	it->chit_ch ?1 '"' == then
	{
		2 it->cs_it_state = ret :s2
		it->cs_it_a ?1 it->chit_ch =1
		it->cs_it_source_it->chit_line ? it->chit_line =
		it->cs_it_source_it->chit_column ? it->chit_column =
		
		loop
		{
			it->chit_ch ?1 '0' == or { it->chit_ch ?1 '"' == } then { break }
			it->chit_ch ?1 '\\' == then
			{
				3 it->cs_it_state = ret :s3
				it->cs_it_source_it ? dup chit_next + ? ()
				it->cs_it_source_it->chit_ch ?1 it->chit_ch =1 
				it->cs_it_source_it->chit_line ? it->chit_line =
				it->cs_it_source_it->chit_column ? it->chit_column =
			}
			4 it->cs_it_state = ret :s4
			it->cs_it_source_it ? dup chit_next + ? ()
			it->cs_it_source_it->chit_ch ?1 it->chit_ch =1 
			it->cs_it_source_it->chit_line ? it->chit_line =
			it->cs_it_source_it->chit_column ? it->chit_column =
		}
		it->chit_ch ?1 '"' == then
		{
			5 it->cs_it_state = ret :s5
			it->cs_it_source_it ? dup chit_next + ? ()
			it->cs_it_source_it->chit_ch ?1 it->cs_it_a =1 
			it->cs_it_source_it->chit_line ? it->chit_line =
			it->cs_it_source_it->chit_column ? it->chit_column =
		}
		0 it->cs_it_state =
		goto s0
	}
	it->chit_ch ?1 '\'' == then
	{
		6 it->cs_it_state = ret :s6
		it->cs_it_a ?1 it->chit_ch =1
		it->cs_it_source_it->chit_line ? it->chit_line =
		it->cs_it_source_it->chit_column ? it->chit_column =
		it->chit_ch ?1 '\\' == then
		{
			7 it->cs_it_state = ret :s7
			it->cs_it_source_it ? dup chit_next + ? ()
			it->cs_it_source_it->chit_ch ?1 it->chit_ch =1 
			it->cs_it_source_it->chit_line ? it->chit_line =
			it->cs_it_source_it->chit_column ? it->chit_column =
		}
		8 it->cs_it_state = ret :s8
		it->cs_it_source_it ? dup chit_next + ? ()
		it->cs_it_source_it->chit_ch ?1 it->chit_ch =1 
		it->cs_it_source_it->chit_line ? it->chit_line =
		it->cs_it_source_it->chit_column ? it->chit_column =
		it->chit_ch ?1 '\'' == then
		{
			9 it->cs_it_state = ret :s9
			it->cs_it_source_it ? dup chit_next + ? ()
			it->cs_it_source_it->chit_ch ?1 it->cs_it_a =1 
			it->cs_it_source_it->chit_line ? it->chit_line =
			it->cs_it_source_it->chit_column ? it->chit_column =
		}
		0 it->cs_it_state =
		goto s0
	}
	ret
}

func new_comment_strip_iterator
{
	var source_it source_it =
	32 sys_malloc() var it it =
	source_it ? it->cs_it_source_it =
	source_it->chit_filename ? it->chit_filename =
	comment_strip_iterator_next it->chit_next =
	source_it->chit_ch ?1 it->cs_it_a =1
	0 it->cs_it_state =
	it ? comment_strip_iterator_next()
	it ?
	ret
}

# include_iterator

const i_it_source_it 20
const i_it_parent_source_its 24 # array of size 10
const i_it_nr_parents 64

func include_iterator_next
{
	var it it =

	it->i_it_source_it ? dup chit_next + ? ()
	it->i_it_source_it->chit_ch ?1 it->chit_ch =1
	it->chit_ch ? '\0' == then
	{
		it->i_it_nr_parents 0 == then
		{
			ret
		}
		'\n' it->chit_ch =1
		it->i_it_nr_parents dup ? 4 - =:
		it->i_it_parent_source_its it->i_it_nr_parents + ? it->i_it_source_it =
		it->i_it_source_it->chit_filename ? it->chit_filename =
	}
	it->i_it_source_it->chit_line ? it->chit_line =
	it->i_it_source_it->chit_column ? it->chit_column =
	ret
}

func include_iterator_add
{
	var include_it include_it =
	var it it =
	
	it->i_it_parent_source_its it->i_it_nr_parents ? + ? it->i_it_source_it =
	it->i_it_nr_parents dup ? 4 + =:
	include_it ? it->i_it_source_it =
	it->i_it_source_it->chit_filename ? it->chit_filename =
	it->i_it_source_it->chit_line ? it->chit_line =
	it->i_it_source_it->chit_column ? it->chit_column =
	it->i_it_source_it->chit_ch ? it->chit_ch =
	ret
}

func new_include_iterator
{
	var include_it include_it =
	
	68 sys_malloc() var it it =
	include_it ? it->i_it_source_it =
	it->i_it_source_it->chit_filename ? it->chit_filename =
	it->i_it_source_it->chit_line ? it->chit_line =
	it->i_it_source_it->chit_column ? it->chit_column =
	it->i_it_source_it->chit_ch ? it->chit_ch =
	include_iterator_next it->chit_next =
	0 it->i_it_nr_parents =
	ret
}
	
###

const TK_D_HASH	 	500
const TK_EQ		 	501
const TK_NE		 	502
const TK_LE		 	503
const TK_GE		 	504
const TK_INC	 	505
const TK_DEC	 	506
const TK_ARROW	 	507
const TK_ASS		600
const TK_DD_OPER	800
const TK_KEYWORD	1000
const TK_CASE		1000
const TK_CHAR		1001
const TK_CONST		1002
const TK_DEFAULT	1003
const TK_DO			1004
const TK_DOUBLE		1005
const TK_ELSE		1006
const TK_ENUM		1007
const TK_EXTERN		1008
const TK_FOR		1009
const TK_GOTO		1010
const TK_IF			1011
const TK_INLINE		1012
const TK_INT		1013
const TK_LONG		1014
const TK_SHORT		1015
const TK_SIZEOF		1016
const TK_STATIC		1017
const TK_STRUCT		1018
const TK_SWITCH		1019
const TK_THEN		1020
const TK_TYPEDEF	1021
const TK_UNION		1022
const TK_UNSIGNED	1023
const TK_VOID		1024
const TK_WHILE		1025
const TK_H_ELSE		1026
const TK_H_ELIF		1027
const TK_H_ENDIF	1028
const TK_H_DEFINE	1029
const TK_DEFINED	1030
const TK_H_IF		1031
const TK_H_IFDEF	1032
const TK_H_IFNDEF	1033
const TK_H_INCLUDE	1034
const TK_H_UNDEF	1035

# token_iterator

const t_it_kind 0
const t_it_token 4
const t_it_int_value 8
const t_it_filename 12
const t_it_line 16
const t_it_column 20
const t_it_next 24

# tokenizer

const tz_at_start_of_line 28
const tz_char_iterator 32

func tokenizer_skip_white_space
{
	var skip_nl skip_nl =
	var tokenizer tokenizer =
	
	tokenizer->tz_char_iterator ? var it it =
	it->chit_ch var ch ch =

	loop
	{
		ch ? '\0' == or { ch ? ' ' <= or { skip_nl ? 0 == and { ch ? '\n' == } } } then { break }
		ch ? '\n' == then
		{
			1 tokenizer->tz_at_start_of_line =
		}
		it ? dup chit_next + ? ()
		it->chit_ch ? ch =
	}
	ret
}

func tokenizer_parse_char_literal
{
	var tokenizer tokenizer =

	tokenizer->tz_char_iterator ? var it it =
	it->chit_ch ? var ch ch =
	it ? dup chit_next + ? ()
	ch ? '\\' != then
	{
		ch ? ret
	}
	it->chit_ch ? ch =
	it ? dup chit_next + ? ()
	ch ? '0' == or { ch ? '1' == } then
	{
		ch '0' - var val val =
		it->chit_ch ? ch =
		'0' ch ? <= and { ch ? '7' <= } then
		{
			val ? 8 * ch ? + '0' - val =
			it ? dup chit_next + ? ()
			it->chit_ch ? ch =
			'0' ch ? <= and { ch ? '7' <= } then
			{
				val ? 8 * ch ? + '0' - val =
				it ? dup chit_next + ? ()
			}
			val ? ret
		}
		val ? 0 then { 0 } else { '1' } ret
	}
	ch ? 'n' == then { '\n' ret }
	ch ? 'r' == then { '\r' ret }
	ch ? ret
}
	
func tokenizer_shift_char
{
	var ref_i ref_i =
	var ref_ch ref_ch =
	var tokenizer tokenizer =
	
	ref_ch ? ? tokenizer->t_it_token ? ref_i ? ? + =1
	ref_i ? ? 1 + ref_i ? =
	tokenizer->tz_char_iterator ? dup chit_next + ? ()
	tokenizer->tz_char_iterator->chit_ch ? ref_ch ? =
}

func tokenizer_next
{
	var skip_nl skip_nl =
	var tokenizer tokenizer =
	
	tokenizer ? skip_nl ? tokenizer_skip_white_space()
	tokenizer->tz_char_iterator ? var it it =
	it->chit_next ? var it_next it_next =
	0 var i i =
	1 var sign sign =
	it->chit_ch ? var ch ch =
	0 tokenizer->t_it_kind =
	ch ? 0 == then { goto done }
	it->chit_filename ? it->t_it_filename =
	it->chit_line ? it->t_it_line =
	it->chit_column ? it->t_it_column =
	ch ? '\n' == then
	{
		'\n' tokenizer->t_it_kind =
		goto done
	}
	tokenizer->tz_at_start_of_line and { ch ? '#' == } then
	{
		tokenizer ? ch i tokenizer_shift_char()
		tokenizer ? 1 tokenizer_skip_white_space()
		it->chit_ch ? ch =
		loop
		{
			ch ? 'a' < then { break }
			ch ? 'z' > then { break }
			tokenizer ? ch i tokenizer_shift_char()
		}
		'\0' ? tokenizer->t_it_token ? i ? + =1
		tokenizer->t_it_token ? "#else"    strcmp() 0 == then { TK_H_ELSE    tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "#elif"    strcmp() 0 == then { TK_H_ELIF    tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "#endif"   strcmp() 0 == then { TK_H_ENDIF   tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "#define"  strcmp() 0 == then { TK_H_DEFINE  tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "#if"      strcmp() 0 == then { TK_H_IF      tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "#ifdef"   strcmp() 0 == then { TK_H_IFDEF   tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "#ifndef"  strcmp() 0 == then { TK_H_IFNDEF  tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "#include" strcmp() 0 == then { TK_H_INCLUDE tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "#undef"   strcmp() 0 == then { TK_H_UNDEF   tokenizer->t_it_kind = goto done }
		goto done
	}
	'a' ch ? <= and { ch ? 'z' <= } or { 'A' ch ? <= and { ch ? 'Z' <= } or { ch ? '_' == } } then
	{
		'i' tokenizer->t_it_kind =
		tokenizer ? ch i tokenizer_shift_char()
		loop
		{
			'a' ch ? <= and { ch ? 'z' <= } or { 'A' ch ? <= and { ch ? 'Z' <= } or { '0' ch ? <= and { ch ? '9' <= } or { ch ? '_' == } } } then
			{
				tokenizer ? ch i tokenizer_shift_char()
			}
			else { break }
		}
		'\0' tokenizer->t_it_token ? i ? + =1
		tokenizer->t_it_token ? "case"     strcmp() 0 == then { TK_CASE     tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "char"     strcmp() 0 == then { TK_CHAR     tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "const"    strcmp() 0 == then { TK_CONST    tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "default"  strcmp() 0 == then { TK_DEFAULT  tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "defined"  strcmp() 0 == then { TK_DEFINED  tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "do"       strcmp() 0 == then { TK_DO       tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "double"   strcmp() 0 == then { TK_DOUBLE   tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "else"     strcmp() 0 == then { TK_ELSE     tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "enum"     strcmp() 0 == then { TK_ENUM     tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "extern"   strcmp() 0 == then { TK_EXTERN   tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "for"      strcmp() 0 == then { TK_FOR      tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "goto"     strcmp() 0 == then { TK_GOTO     tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "if"       strcmp() 0 == then { TK_IF       tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "inline"   strcmp() 0 == then { TK_INLINE   tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "int"      strcmp() 0 == then { TK_INT      tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "long"     strcmp() 0 == then { TK_LONG     tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "short"    strcmp() 0 == then { TK_SHORT    tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "sizeof"   strcmp() 0 == then { TK_SIZEOF   tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "static"   strcmp() 0 == then { TK_STATIC   tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "struct"   strcmp() 0 == then { TK_STRUCT   tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "switch"   strcmp() 0 == then { TK_SWITCH   tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "then"     strcmp() 0 == then { TK_THEN     tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "typedef"  strcmp() 0 == then { TK_TYPEDEF  tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "union"    strcmp() 0 == then { TK_UNION    tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "unsigned" strcmp() 0 == then { TK_UNSIGNED tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "void"     strcmp() 0 == then { TK_VOID     tokenizer->t_it_kind = goto done } 
		tokenizer->t_it_token ? "while"    strcmp() 0 == then { TK_WHILE    tokenizer->t_it_kind = goto done } 
		goto done	
	}
	'0' ch ? <= and { ch ? '9' <= } or { ch ? '-' == } then
	{
		ch ? '-' == then
		{
			tokenizer ? ch i tokenizer_shift_char()
			ch ? '-' == then
			{
				TK_DEC tokenizer->t_it_kind =
				tokenizer ? ch i tokenizer_shift_char()
				goto done
			}
			ch ? "=" == then
			{
				TK_ASS '-' + tokenizer->t_it_kind =
				tokenizer ? ch i tokenizer_shift_char()
				goto done
			}
			ch ? ">" == then
			{
				TK_ARROW tokenizer->t_it_kind =
				tokenizer ? ch i tokenizer_shift_char()
				goto done
			}
			ch ? '0' < or { '9' ch ? < } then
			{
				's' tokenizer->t_it_kind =
				goto done
			}
			-1 sign =
		}
		'0' tokenizer->t_it_kind =
		0 tokenizer->t_it_int_value =
		ch ? '0' == then
		{
			tokenizer ? ch i tokenizer_shift_char()
			ch ? 'x' == then
			{
				tokenizer ? ch i tokenizer_shift_char()
				loop
				{
					'0' ch ? <= and { ch ? '9' <= } then
					{
						ch ? '0' -
					}
					else
					{
						'a' ch ? <= and { ch ? 'f' <= } then
						{
							ch ? 'a' - 10 +
						}
						else
						{
							'A' ch ? <= and { ch ? 'F' <= } then
							{
								ch ? 'A' - 10 +
							}
							else
							{
								break
							}
						}
					}
				}
				tokenizer->t_it_int_value ? 16 * + tokenizer->t_it_int_value =
				tokenizer ? ch i tokenizer_shift_char()
			}
			else
			{
				loop
				{
					'0' ch ? > or { ch ? '7' > } then { break }
					ch ? '0' tokenizer->t_it_int_value ? 8 * + tokenizer->t_it_int_value =
					tokenizer ? ch i tokenizer_shift_char()
				}
			}
		}
		else
		{
			loop
			{
				'0' ch ? > or { ch ? '9' > } then { break }
				ch ? '0' tokenizer->t_it_int_value ? 10 * + tokenizer->t_it_int_value =
				tokenizer ? ch i tokenizer_shift_char()
			}
		}
		ch ? 'U' == then
		{
			tokenizer ? ch i tokenizer_shift_char()
		}
		loop
		{
			ch ? 'L' != then { break }
			tokenizer ? ch i tokenizer_shift_char()
		}
		tokenizer->t_it_int_value dup ? sign ? * =:
		goto done 
	}
	ch ? '\'' == then
	{
		ch ? tokenizer->t_it_kind =
		tokenizer->tz_char_iterator ? dup chit_next + ? ()
		tokenizer ? tokenizer_parse_char_literal() tokenizer->t_it_token =
		1 i =
		it->chit_ch '\'' == then
		{
			tokenizer->tz_char_iterator ? dup chit_next + ? ()
		}
		goto done
	}
	ch ? '"' == then
	{
		ch ? tokenizer->t_it_kind =
		tokenizer->tz_char_iterator ? dup chit_next + ? ()
		loop
		{
			ch ? '\0' == or { ch ? '"' == } then { break }
			tokenizer ? tokenizer_parse_char_literal() tokenizer->t_it_token =
			ch? tokenizer->t_it_token ? i ? + =1
			i ? 1 + i =
		}
		it->chit_ch '"' == then
		{
			tokenizer->tz_char_iterator ? dup chit_next + ? ()
		}
		goto done
	}
	ch ? tokenizer->t_it_kind =
	ch ? '#' == then
	{
		tokenizer ? ch i tokenizer_shift_char()
		ch ? '#' == then
		{
			TK_D_HASH tokenizer->t_it_kind =
			tokenizer ? ch i tokenizer_shift_char()
		}
		goto done
	}
	ch ? '=' == then
	{
		tokenizer ? ch i tokenizer_shift_char()
		ch ? '=' == then
		{
			TK_EQ tokenizer->t_it_kind =
			tokenizer ? ch i tokenizer_shift_char()
		}
		goto done
	}
	ch ? '*' == or { ch ? '/' == or { ch ? '%' == or { ch ? '^' == } } } then
	{
		ch ? var fch fch =
		tokenizer ? ch i tokenizer_shift_char()
		ch ? '=' == then
		{
			fch ? TK_ASS + tokenizer->t_it_kind =
			tokenizer ? ch i tokenizer_shift_char()
		}
		goto done		
	}
	ch ? '+' == or { ch ? '-' == } then
	{
		ch ? var fch fch =
		tokenizer ? ch i tokenizer_shift_char()
		ch ? '=' == then
		{
			fch ? TK_ASS + tokenizer->t_it_kind =
			tokenizer ? ch i tokenizer_shift_char()
			goto done
		}
		fch ? ch ? == then
		{
			ch ? '+' == then { TK_INC } else { TK_DEC } tokenizer->t_it_kind =
			tokenizer ? ch i tokenizer_shift_char()
		}
		goto done		
	}
	ch ? '<' == or { ch ? '>' == or { ch ? '|' == or { ch ? '&' == } } } then
	{
		ch ? var fch fch =
		tokenizer ? ch i tokenizer_shift_char()
		fch ? ch ? == then
		{
			TK_DD_OPER ch ? + tokenizer->t_it_kind =
			tokenizer ? ch i tokenizer_shift_char()
		}
		ch ? '=' == and { fch ? '<' == or { fch ? '>' == } } then
		{
			i ? 2 == then { TK_ASS ch ? + } else { fch ? '<' == then { TK_LE } else { TK_GE } } tokenizer->t_it_kind =
			tokenizer ? ch i tokenizer_shift_char()
		}
		goto done
	}
	ch ? '!' == then
	{
		tokenizer ? ch i tokenizer_shift_char()
		ch ? '=' == then
		{
			TK_NE + tokenizer->t_it_kind =
			tokenizer ? ch i tokenizer_shift_char()
		}
		goto done
	}
	tokenizer ? ch i tokenizer_shift_char()
	
	:done
	0 tokenizer->t_it_token ? i ? + =1
}

func new_tokenizer
{
	var char_iterator char_iterator =
	36 sys_malloc() var tokenizer tokenizer =
	1 tokenizer->tz_at_start_of_line =
	char_iterator ? tokenizer->tz_char_iterator =
	6000 sys_malloc() tokenizer->t_it_token =
	tokenizer_next tokenizer->t_it_next =
	tokenizer ret
}

#####

func echo_test_sl
{
	"test.sl" 0 0 sys_open() var fin fin =
	loop
	{
		fin ? sys_fgetc() var ch ch =
		ch ? 0 == then { break }
		ch ? 1 sys_fputc() pop
	}
	fin ? sys_close()
	ret
}

func test_f_iter
{
	"test.sl" new_file_iterator () var file_it file_it =
	loop
	{
		file_it ? chit_ch + ? 0 == then { break }
		file_it ? chit_ch + ? stdout sys_fputc() pop
		file_it ? dup chit_next + ? ()
	}
	ret
}

func test_sl
{
	"test_sl.c" new_file_iterator() new_line_splice_iterator() new_comment_strip_iterator() var file_it file_it =
	loop
	{
		file_it ? chit_ch + ? 0 == then { break }
		file_it ? chit_ch + ? stdout sys_fputc() pop
		file_it ? dup chit_next + ? ()
	}
	ret
}

func test_token_iter
{
	"test.c" new_file_iterator() new_line_splice_iterator() new_comment_strip_iterator() var file_it file_it =
	file_it ? new_tokenizer() var token_it token_it =
	
	loop
	{
		token_it ? token_it->t_it_next ? ()
		token_it->t_it_kind ? 0 == then { break }
		token_it->t_it_kind ? 1 sys_fputc()
		" : '" 1 fhputs()
		token_it->t_it_token ? 1 fhputs()
		"'\n" 1 fhputs()
	}
}

func main
{
	2 1 > then { "2 > 1\n" } else { "Error: 2 > 1\n" } stdout fhputs()
	12 10 * stdout fhput_int()
	
	"\nhello world!\n" copystr() stdout fhputs()
	"" "" strcmp() stdout fhput_int() "\n" stdout fhputs()
	"a" "b" strcmp() stdout fhput_int() "\n" stdout fhputs()
	"b" "a" strcmp() stdout fhput_int() "\n" stdout fhputs()
	"a" "a" strcmp() stdout fhput_int() "\n" stdout fhputs()
	"a" "ab" strcmp() stdout fhput_int() "\n" stdout fhputs()
	# test_f_iter()
	#test_sl()
	test_token_iter()
	ret
}