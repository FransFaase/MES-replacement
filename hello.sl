
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

func main {
	"hello world!" fhputs ()
	ret
}