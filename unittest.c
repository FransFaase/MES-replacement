#ifdef __GNUC__
#include <stdio.h>
#include <string.h>
#endif
#ifdef __TINYC__
typedef int size_t;
#include "stdlib.c"
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

int switch_wd(int a)
{
	int r = 0;
	switch (a++)
	{
		case 1:
			r = 1;
		default:
			r += 10;
		case 2:
			r += 100;
	}
	return r;
}

int switch_nod(int a)
{
	int r = 0;
	switch(a++)
	{
		case 0:
			r = 1;
			break;
		case 1:
			r = 20;
		case 2:
			r += 100;
			break;
		case 3:
			r += 4;
			break;
	}
	return r;
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
	
	int xx = 1;
	int y = xx + 1;

#ifdef __TINYC__
	printf("%s\n", TCC_VERSION);
#endif

#define STR "teststr"
	printf("%s\n", STR);

	char *char_p1 = (char*)malloc(20 * sizeof(char));
	char *char_p2 = (char*)malloc(20 * sizeof(char));
	char *char_p3 = &char_p1[3];
	is_true(char_p1 < char_p3 && char_p3 < char_p1 + 10, "p3 in p1");
	is_true(char_p2 > char_p1, "char_p2 > char_p1");

	int zz;
	sscanf("3", "%d",&zz);
	is_true(zz == 3, "sscanf 3");

	is_true(1000 + switch_wd(1) == 1111, "switch_wd(1)");
	is_true(1000 + switch_wd(2) == 1100, "switch_wd(2)");
	is_true(1000 + switch_wd(0) == 1110, "switch_wd(0)");
	is_true(1000 + switch_wd(3) == 1110, "switch_wd(3)");

	is_true(1000 + switch_nod(0) == 1001, "switch_nod(0)");
	is_true(1000 + switch_nod(1) == 1120, "switch_nod(1)");
	is_true(1000 + switch_nod(2) == 1100, "switch_nod(2)");
	is_true(1000 + switch_nod(3) == 1004, "switch_nod(3)");
	is_true(1000 + switch_nod(4) == 1000, "switch_nod(4)");
	is_true(1000 + switch_nod(40) == 1000, "switch_nod(4)");

	printf("Done\n");

	return result;
}
