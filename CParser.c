#include <unistd.h>
#include <fcntl.h>

//#define TRACE_ALLOCATIONS 1
//#define CHECK_LOCAL_RESULT
//#define SAFE_CASTING

#define INCLUDED
#include "../RawParser/src/RawParser.c"


int main(int argc, char *argv[])
{
	//debug_parse = TRUE;
	//debug_nt = TRUE;
	
	file_ostream_t debug_ostream;
	file_ostream_init(&debug_ostream, stdout);
	stdout_stream = &debug_ostream.ostream;
	
	non_terminal_dict_p all_nt = NULL;

	white_space_grammar(&all_nt);
	ident_grammar(&all_nt);
	char_grammar(&all_nt);
	string_grammar(&all_nt);
	int_grammar(&all_nt);
	c_grammar(&all_nt);
	
	int fh = open("tcc.c", O_RDONLY);
	if (fh < 0)
	{
		fprintf(stderr, "Failed to open out.txt\n");
		return 0;
	}
	unsigned long len = lseek(fh, 0L, SEEK_END);
	lseek(fh, 0L, SEEK_SET);
	char *input = MALLOC_N(len+1, char);
	len = read(fh, input, len);
	input[len] = '\0';

	text_buffer_t text_buffer;
	text_buffer_assign_string(&text_buffer, input);
	
	solutions_t solutions;
	solutions_init(&solutions, &text_buffer);

	init_expected();
	
	parser_t parser;
	parser_init(&parser, &text_buffer);
	parser.cache_hit_function = solutions_find;
	parser.cache = &solutions;
	
	ENTER_RESULT_CONTEXT

	DECL_RESULT(result);
	if (!parse_nt(&parser, find_nt("root", &all_nt), &result) || !text_buffer_end(&text_buffer))
	{
		print_expected(stdout);
	}
	else
	{
		if (result.data == NULL)
			fprintf(stderr, "ERROR: parsing did not return result\n");
		else
		{
			char output[200];
			fixed_string_ostream_t fixed_string_ostream;
			fixed_string_ostream_init(&fixed_string_ostream, output, 200);
			result_print(&result, &fixed_string_ostream.ostream);
			fixed_string_ostream_finish(&fixed_string_ostream);
		}
	}
	DISP_RESULT(result);
	
	solutions_free(&solutions);

	EXIT_RESULT_CONTEXT

	return 0;
}

