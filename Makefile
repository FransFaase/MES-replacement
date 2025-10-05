all : tcc_cc stack_c run_unittest_s stack_c_s stack_c_diff tcc_cc_s tcc_cc_diff diff3 diff5 diff4 tcc_s

% : %.c
	./run.sh gcc $(basename $@)

%.sl : %.c tcc_cc stdlib.c
	./tcc_cc $< -o $@

%_t.sl : %.c tcc_cc_s stdlib.c
	./tcc_cc_s $< -o $@

%_s.M1 : %.sl stack_c stack_c_intro.M1
	./stack_c $< -o $@

%.blood_elf : %.M1
	./blood-elf --file $< --little-endian --output $@

%.macro : %.blood_elf
	./M1 --file $(basename $@).M1 --little-endian --architecture x86 --file $< --output $@

% : %.macro
	./hex2 --file ./M2libc/x86/ELF-x86-debug.hex2 --file $< --output $@ --architecture x86 --base-address 0x8048000 --little-endian 
	objdump -d $@ >$@.txt

run_unittest_s : unittest_s
	./unittest_s

stack_c_diff : stack_c stack_c_s
	./stack_c_s stack_c.sl -o stack_c_s2.M1
	./run.sh diff stack_c_s.M1 stack_c_s2.M1

tcc_cc_diff : tcc_cc tcc_cc_s
	./stack_c_s tcc_cc.sl -o tcc_cc_s2.M1
	./run.sh diff tcc_cc_s.M1 tcc_cc_s2.M1
	
diff3 : unittest.sl unittest_t.sl
	./run.sh diff unittest.sl unittest_t.sl

diff4 : stack_c.sl stack_c_t.sl
	./run.sh diff stack_c.sl stack_c_t.sl

diff5 : tcc_cc.sl tcc_cc_t.sl
	./run.sh diff tcc_cc.sl tcc_cc_t.sl

tcc.sl : tcc_cc tcc_sources/tcc.c
	./tcc_cc -DONE_SOURCE=1 -DTCC_VERSION=\"0.9.26\" -DBOOTSTRAP=1 -DTCC_TARGET_I386=1 -DHAVE_LONG_LONG=0 tcc_sources/tcc.c -o tcc.sl

tcc: tcc_sources/tcc.c
	gcc -DONE_SOURCE=1 -DTCC_VERSION=\"0.9.26\" -DBOOTSTRAP=1 -DTCC_TARGET_I386=1 -DHAVE_LONG_LONG=0 tcc_sources/tcc.c -o tcc

x : tcc_s tcc

.PRECIOUS: %.sl %_s.M1 %.blood_elf %.macro
.PHONY: all run_unittest_s stack_c_diff tcc_cc_diff x
