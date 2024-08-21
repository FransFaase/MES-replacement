# test

const one 1
func strcmp
{
	var b b =
	var a a =
	var result
    loop
    {
		a ? ?1 b ? ?1 - result = 
		result 0 != then { break }
		a ? one + a =
	}
	result ?
	ret
	-1 0xfe
}

func test
{
	1 then
	{
		var x 1 x =
	}
	else
	{
		var y 2 y =
	}
	0 then
	{
		var z 3 z =
	}
	ret
}	

func fhputs {
	var s s =
	loop
	{
		s ? ?1 0 == then { break }
		s ? ?1 1 sys_fputc () pop
		s ? 1 + s =
	}
	ret
}

func fhput_int {
	var f f =
	var i i =
	i ? 0 == then
	{
		'0' f ? sys_fputc() pop
		ret
	}
	i ? 0 < then
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

func main {
	2 1 > then { "2 > 1\n" } else { "Error: 2 > 1\n" } fhputs ()
	12 1 fhput_int ()
	
	"\nhello world!\n" fhputs ()

	"test.sl" 0 0 sys_open () var fin fin =
	loop
	{
		#"start loop\n" fhputs ()
		fin ? sys_fgetc () var ch ch =
		#ch ? 1 fhput_int () 
		#"\n\n" fhputs ()
		#ch ? 0 == 1 fhput_int () 
		#"\n\n" fhputs ()
		ch ? 0 == then
		{ 
			#"break\n" fhputs ()
			break 
		}
		#"no break\n" fhputs ()
		
		ch ? 1 sys_fputc () pop
		#"end loop\n" fhputs ()
	}
	# "out loop\n" fhputs ()
	fin ? sys_close ()
	ret
}