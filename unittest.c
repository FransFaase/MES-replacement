
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
	return 1;
}
