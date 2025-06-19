tcc_cc : tcc_cc.c
	./run.sh c_tcc_cc

stack_c : stack_c.c
	./run.sh c_stack_c

unittest : tcc_cc stack_c stdlib.c stack_c_intro.M1 unittest.c
	./run.sh unittest

stack_c2 : tcc_cc stack_c stdlib.c stack_c_intro.M1 stack_c.c
	./run.sh stack_c2

tcc_cc2 : tcc_cc stack_c stdlib.c stack_c_intro.M1 tcc_cc.c
	./run.sh tcc_cc2

.PHONY : all
all : unittest stack_c2 tcc_cc2
