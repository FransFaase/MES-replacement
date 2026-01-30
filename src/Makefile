all : hex2 M1 blood-elf tcc_cc stack_c run_unittest_s stack_c_s stack_c_diff tcc_cc_s tcc_cc_diff diff3 diff5 diff4 \
      hex0_s hex0_s.hex0 hex2_s.hex0 \
	  blood-elf_s.macro blood-elf_s.blood_elf \
	  M1_s.macro M1_s.blood_elf \
	  stack_c_s.M1 tcc_cc.sl \
	  kaem-minimal_s kaem-minimal_s.hex0 \
	  kaem_s \
	  catm_s \
	  match_s \
	  mkdir_s cp_s chmod_s rm_s \
	  sha256sum_s
 

% : %.c
	./run.sh gcc $(basename $@)

hex0.sl : hex0.c tcc_cc
	./tcc_cc -o $@ $<

hex2.sl : hex2.c tcc_cc
	./tcc_cc -o $@ $<

kaem-minimal.sl : kaem-minimal.c tcc_cc
	./tcc_cc -o $@ $<

%.sl : %.c tcc_cc stdlib.c
	./tcc_cc -o $@ stdlib.c $<

%_t.sl : %.c tcc_cc_s stdlib.c
	./tcc_cc_s -o $@ stdlib.c $< 

%_s.M1 : %.sl stack_c stack_c_intro.M1
	./stack_c -i stack_c_intro.M1 $< -o $@

%.blood_elf : %.M1
	./blood-elf --file $< --little-endian --output $@

%.macro : %.blood_elf
	./M1 $(basename $@).M1 -o $@

% : %.macro
	./hex2 -o $@ ../M2libc/x86/ELF-x86-debug.hex2 $(basename $@).macro $(basename $@).blood_elf
	objdump -d $@ >$@.txt

%.hex0 : %.macro
	./hex2 -o $@ ../M2libc/x86/ELF-x86-debug.hex2 $(basename $@).macro $(basename $@).blood_elf

run_unittest_s : unittest_s stack_c_interpreter
	./unittest_s
	./stack_c_interpreter unittest.sl

stack_c_diff : stack_c stack_c_s
	./stack_c_s -i stack_c_intro.M1 stack_c.sl -o stack_c_s2.M1
	./run.sh diff stack_c_s.M1 stack_c_s2.M1

tcc_cc_diff : tcc_cc tcc_cc_s
	./stack_c_s -i stack_c_intro.M1 tcc_cc.sl -o tcc_cc_s2.M1
	./run.sh diff tcc_cc_s.M1 tcc_cc_s2.M1
	
diff3 : unittest.sl unittest_t.sl
	./run.sh diff unittest.sl unittest_t.sl

diff4 : stack_c.sl stack_c_t.sl
	./run.sh diff stack_c.sl stack_c_t.sl

diff5 : tcc_cc.sl tcc_cc_t.sl
	./run.sh diff tcc_cc.sl tcc_cc_t.sl


.PRECIOUS: %.sl %_s.M1 %.blood_elf %.macro
.PHONY: all run_unittest_s stack_c_diff tcc_cc_diff
