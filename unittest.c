#ifdef __GNUC__
#include <stdio.h>
#include <string.h>
#endif

int result = 0;

void is_true(int val, const char *testname)
{
	if (!val)
	{
		printf("Failed: %s\n", testname);
		result = 1;
	}
}

void is_false(int val, const char *testname)
{
	if (val)
	{
		printf("Failed: %s\n", testname);
		result = 1;
	}
}

int one(void)
{
	return 1;
}

void void_func(void)
{

}

typedef struct with_func_pointer_s *with_func_pointer_p;
struct with_func_pointer_s
{
	int (*func)(void);
};

with_func_pointer_p new_struct_with_function_pointer(void)
{
	with_func_pointer_p it = malloc(sizeof(struct with_func_pointer_s));
	it->func = one;
	return it;
}


int main (int argc, char *argv[])
{
	printf("argc = %d\n", argc);
	for (int i = 0; i < argc; i++)
		printf("%d %s\n", i, argv[i]);
	printf("\n\n\n");
	is_true(1, "1");
	is_true(1 == 1, "1 == 1");
	is_true(1 << 2 == 4, "1 << 2 == 4");
	is_true(4 >> 2 == 1, "4 >> 2 == 1");
	int a = 8;
	a >>= 2;
	is_true(a == 2, "int a = 8; a >>= 2; a == 2");
	is_true(4 / 2 == 2, "4 / 2 == 2");
	is_true(44 / 10 == 4, "44 / 10 == 4");
	is_true(1 / 10 == 0, "1 / 10 == 0");
	is_true(-5 < 0, "-5 < 0");
	is_true(0 < 4, "0 < 4");
	printf("(%d %d) == (44 -44)\n", 44, -44);
	char buffer[40];
	snprintf(buffer, 30, "(%d %d)", 123, -78);
	is_true(strcmp(buffer, "(123 -78)") == 0, "sprintf 123 -78");
	is_true(strcmp("A","A") == 0, "A = A");
	char *mode = "r";
	is_true(mode[0] == 'r', "mode[0] = r");
	const char *eln = "\\n";
	is_true(eln[0] == '\\', "first is \\");
	is_true(eln[1] == 'n', "second is n");
	snprintf(buffer, 30, "%s", eln);
	is_true(buffer[0] == '\\', "b first is \\");
	is_true(buffer[1] == 'n', "b second is n");
	is_true(strcmp(buffer, "\\n") == 0, "sprintf \\\\n");
	is_true((5 & 3) == 1, "(5 & 3) == 1");
	is_true((5 | 3) == 7, "(5 | 3) == 7");
	is_true(-2 * 3 == -6, "-2 * 3 == -6");
	is_true(2 * -3 == -6, "2 * -3 == -6");
	is_true(-2 * -3 == 6, "-2 * -3 == 6");
	is_true(4 / 2 == 2, "4 / 2 == 2");
	is_true(-4 / 2 == -2, "-4 / 2 == -2");
	is_true(4 / -2 == -2, "4 / -2 == -2");
	is_true(-4 / -2 == 2, "-4 / -2 == 2");
	unsigned int negu = -1;
	is_true(negu > 0, "negu > 0");
	int (*func)(void) = one;
	int x = func();
	is_true(x == 1, "func() == 1");
	with_func_pointer_p it_one = new_struct_with_function_pointer();
	is_true(it_one->func() == 1, "it_one->func() == 1");
	void (*v_func)(void) = void_func;
	v_func();
	int array[] = { 1, 2, 3 };
	is_true(sizeof(array) == 3 * sizeof(array[0]), "size of array is 3");
	static int s_int = 1;
	is_true(s_int == 1, "static int == 1");

	return result;
}
