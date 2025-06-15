
void is_true(int val, const char *testname)
{
	if (!val)
	{
		printf("Failed: %s\n", testname);
	}
}

void is_false(int val, const char *testname)
{
	if (val)
	{
		printf("Failed: %s\n", testname);
	}
}

int main (int argc, char *argv[])
{
	printf("\n\n\n");
	is_true(1, "1");
	is_true(1 == 1, "1 == 1");
	is_true(1 << 2 == 4, "1 << 2 == 4");
	is_true(4 >> 2 == 1, "4 >> 2 == 1");
	is_true(4 / 2 == 2, "4 / 2 == 2");
	is_true(44 / 10 == 4, "44 / 10 == 4");
	is_true(1 / 10 == 0, "1 / 10 == 0");
	is_true(-5 < 0, "-5 < 0");
	is_true(0 < 4, "0 < 4");
	printf("(%d %d)\n", 44, -44);
	char buffer[40];
	snprintf(buffer, 30, "(%d)", 123);
	is_true(strcmp(buffer, "(123)") == 0, "sprintf 123");
	is_true(strcmp("A","A") == 0, "A = A");
	char *mode = "r";
	is_true(mode[0] == 'r', "mode[0] = r");
	printf("argc = %d\n", argc);
	for (int i = 0; i < argc; i++)
		printf("%d %s\n", i, argv[i]);
	return 1;
}
