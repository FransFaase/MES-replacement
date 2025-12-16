#ifdef __GNUC__
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
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
	switch (a++)
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

int switch_label(int a, int b, const char *testname)
{
	int r = 0;
	switch (a)
	{
		case 0:
			r = 1;
			goto _default;
		case 1:
			r = 2;
			break;
		case 3:
			r = 3;
			break;
			_default :
		default:
			r += 4;
	}
	if (r != b)
	{
		printf("Failed: %s\n", testname);
		result = 1;
	}

	return r;
}

typedef struct {
	int x;
	int y;
} coord_t;

typedef struct {
	short a;
	short b;
} Elf32_Test;

typedef union {
	int i;
	Elf32_Test p;
} test_union;

typedef struct {
	union {
		struct {
			int a, b;
		};
		struct {
			int c, d;
		};
	};
	int x;
} nested_union_t;

int compare_ints(const void *a, const void *b)
{
	return *(int*)a - *(int*)b;
}

void test_printf(const char *format, int value, const char *expect, const char *err)
{
	char buffer[40];
	snprintf(buffer, 39, format, value);
	is_true(strcmp(buffer, expect) == 0, err);
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
	is_true(45456 >> 32 == 0, "45456 >> 32 == 0");
	int a = 8;
	a >>= 2;
	is_true(a == 2, "int a = 8; a >>= 2; a == 2");
	is_true(4 / 2 == 2, "4 / 2 == 2");
	is_true(44 / 10 == 4, "44 / 10 == 4");
	is_true(1 / 10 == 0, "1 / 10 == 0");
	is_true(-5 < 0, "-5 < 0");
	is_true(0 < 4, "0 < 4");
	printf("(%d %d) == (44 -44)\n", 44, -44);
	char signed_char = 255;
	is_true(signed_char == -1, "signed_char = -1");
	unsigned char unsigned_char = -1;
	is_true(unsigned_char == 255, "unsigned_char = 255");
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

	switch_label(0, 5, "switch_label(0) == 5");
	switch_label(1, 2, "switch_label(1) == 2");
	switch_label(2, 4, "switch_label(2) == 4");
	switch_label(3, 3, "switch_label(3) == 3");
	switch_label(4, 4, "switch_label(4) == 4");

	int *ref = &zz;
	*ref = 4;
	is_true(zz == 4, "zz == 4");

	coord_t coord_a, coord_b, coord_c;
	coord_a.x = 1;
	coord_a.y = 2;
	coord_b.x = coord_b.y = coord_c.x = coord_c.y = 0;
	is_true(coord_b.x == 0, "coord_b.x == 0");
	is_true(coord_b.y == 0, "coord_b.y == 0");
	coord_c = coord_b = coord_a;
	is_true(coord_b.x == 1, "coord_b.x == 1");
	is_true(coord_b.y == 2, "coord_b.y == 2");
	is_true(coord_c.x == 1, "coord_c.x == 1");
	is_true(coord_c.y == 2, "coord_c.y == 2");

	coord_t *ref_coord = &coord_a;
	coord_b.x = 5;
	*ref_coord = coord_b;
	is_true(coord_a.x == 5, "coord_a.x == 5");

	test_union tu;
	tu.i = 0x10002;
	is_true(tu.i == 0x10002, "tu.i == 0x10002");
	is_true(tu.p.a == 2, "tu.p.a == 2");
	is_true(tu.p.b == 1, "tu.p.b == 1");
	tu.p.a = 3;
	is_true(tu.p.a == 3, "tu.p.a == 2");
	is_true(tu.p.b == 1, "tu.p.a == 2");
	is_true(tu.i == 0x10003, "tu.i == 0x10002");
	tu.p.b = 4;
	is_true(tu.p.a == 3, "tu.p.a == 2");
	is_true(tu.p.b == 4, "tu.p.a == 2");
	is_true(tu.i == 0x40003, "tu.i == 0x10002");
	tu.p.a = 5;
	is_true(tu.p.a == 5, "tu.p.a == 2");
	is_true(tu.p.b == 4, "tu.p.a == 2");
	is_true(tu.i == 0x40005, "tu.i == 0x10002");

	int array2[5] = { 5, 1, 2, 4, 3 };
	qsort(array2, 5, sizeof(int), compare_ints);
	//for (int i = 0; i < 5; i++)
	//	printf("array2[%d] = %d\n", i, array2[i]);
	is_true(array2[0] == 1, "array2[0] == 1");
	is_true(array2[1] == 2, "array2[1] == 2");
	is_true(array2[2] == 3, "array2[2] == 3");
	is_true(array2[3] == 4, "array2[3] == 4");
	is_true(array2[4] == 5, "array2[4] == 5");

	char bytes[2];
	bytes[1] = 2;
	void *ptr_byte = bytes + 1;
	*(char*)ptr_byte *= 4;
	is_true(bytes[1] == 8, "byte == 8");

	test_printf("%d", 2, "2", "printf %d 2");
	test_printf("%d", 123, "123", "printf %d 123");
	test_printf("%-5d", 123, "123  ", "printf %-5d 123");
	test_printf("%5d", 123, "  123", "printf %5d 123");
	test_printf("%-2d", 123, "12", "printf %-2d 123");
	test_printf("%2d", 123, "12", "printf %2d 123");

	nested_union_t nu;
	nu.a = 3;
	nu.b = 4;
	nu.x = 5;
	is_true(nu.a == 3, "nu.a == 3");
	is_true(nu.b == 4, "nu.b == 4");
	is_true(nu.c == 3, "nu.c == 3");
	is_true(nu.d == 4, "nu.d == 4");
	is_true(nu.x == 5, "nu.x == 5");

	printf("Done\n");

	return result;
}
