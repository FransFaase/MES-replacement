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
			it->cs_it_source_it ? dup chit_next ? ()
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
		it->cs_it_source_it ? dup chit_next ? ()
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
}

func new_comment_strip_iterator
{
	var source_it source_it =
	32 sys_malloc() var it it =
	source_it ? it->cs_it_source_it =
	#source_it->chit_filename ? 
}

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
	"test_sl.c" new_file_iterator() new_line_splice_iterator() var file_it file_it =
	loop
	{
		file_it ? chit_ch + ? 0 == then { break }
		file_it ? chit_ch + ? stdout sys_fputc() pop
		file_it ? dup chit_next + ? ()
	}
	ret
}

func main
{
	2 1 > then { "2 > 1\n" } else { "Error: 2 > 1\n" } stdout fhputs()
	12 stdout fhput_int()
	
	"\nhello world!\n" copystr() stdout fhputs()
	"" "" strcmp() stdout fhput_int() "\n" stdout fhputs()
	"a" "b" strcmp() stdout fhput_int() "\n" stdout fhputs()
	"b" "a" strcmp() stdout fhput_int() "\n" stdout fhputs()
	"a" "a" strcmp() stdout fhput_int() "\n" stdout fhputs()
	"a" "ab" strcmp() stdout fhput_int() "\n" stdout fhputs()
	# test_f_iter()
	test_sl()
	ret
}