all : hex2 M1 blood-elf tcc_cc stack_c x86 amd64
x86 : unittest_s_OK stack_c_s stack_c_diff_OK tcc_cc_s tcc_cc_diff_OK diff3_OK diff5_OK diff4_OK \
      hex0_s hex0_s.hex0 hex2_s.hex0 \
	  blood-elf_s.macro blood-elf_s.blood_elf \
	  M1_s.macro M1_s.blood_elf \
	  stack_c_s.M1 tcc_cc.sl \
	  kaem-minimal_s kaem-minimal_s.hex0 \
	  kaem_s \
	  catm_s \
	  match_s \
	  mkdir_s cp_s chmod_s rm_s \
	  untar_s ungz_s unxz_s unbz2_s \
	  configurator_s script-generator_s \
      sha256sum_s \
	  equal_s

% : %.c
	./run.sh gcc $(basename $@)

hex0.sl : hex0.c tcc_cc
	./tcc_cc -o $@ $<

hex2.sl : hex2.c tcc_cc
	./tcc_cc -o $@ $<

kaem-minimal.sl : kaem-minimal.c tcc_cc
	./tcc_cc -o $@ $<

equal.sl : equal.c tcc_cc
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
	objdump -d -M intel $@ >$@.txt

%.hex0 : %.macro
	./hex2 -o $@ ../M2libc/x86/ELF-x86-debug.hex2 $(basename $@).macro $(basename $@).blood_elf

unittest_s_OK : unittest_s stack_c_interpreter
	./unittest_s
	./stack_c_interpreter unittest.sl

stack_c_diff_OK : stack_c stack_c_s
	./stack_c_s -i stack_c_intro.M1 stack_c.sl -o stack_c_s2.M1
	./run.sh diff stack_c_s.M1 stack_c_s2.M1 stack_c_diff_OK

tcc_cc_diff_OK : tcc_cc tcc_cc_s
	./stack_c_s -i stack_c_intro.M1 tcc_cc.sl -o tcc_cc_s2.M1
	./run.sh diff tcc_cc_s.M1 tcc_cc_s2.M1 tcc_cc_diff_OK
	
diff3_OK : unittest.sl unittest_t.sl
	./run.sh diff unittest.sl unittest_t.sl diff3_OK

diff4_OK : stack_c.sl stack_c_t.sl
	./run.sh diff stack_c.sl stack_c_t.sl diff4_OK

diff5_OK : tcc_cc.sl tcc_cc_t.sl
	./run.sh diff tcc_cc.sl tcc_cc_t.sl diff5_OK


amd64 : stack_c_amd64 unittest.amd64_OK stack_c_diff_amd64_OK tcc_cc_s tcc_cc_diff_amd64_OK diff3_amd64_OK diff5_amd64_OK diff4_amd64_OK \
      hex0.amd64 hex0.amd64_hex0 hex2.amd64_hex0 \
	  blood-elf.macro_amd64 blood-elf.blood_elf_amd64 \
	  M1.macro_amd64 M1.blood_elf_amd64 \
	  stack_c_amd64.M1_amd64 tcc_cc.sl64 \
	  kaem-minimal.amd64 kaem-minimal.amd64_hex0 \
	  kaem.amd64 \
	  catm.amd64 \
	  match.amd64 \
	  mkdir.amd64 cp.amd64 chmod.amd64 rm.amd64 \
	  untar_s ungz.amd64 unxz.amd64 unbz2.amd64 \
	  configurator.amd64 script-generator.amd64 \
      sha256sum.amd64 \
	  equal.amd64

hex0.sl64 : hex0.c tcc_cc
	./tcc_cc -64 -o $@ $<

hex2.sl64 : hex2.c tcc_cc
	./tcc_cc -64 -o $@ $<

kaem-minimal.sl64 : kaem-minimal.c tcc_cc
	./tcc_cc -64 -o $@ $<

equal.sl64 : equal.c tcc_cc
	./tcc_cc -64 -o $@ $<

%.sl64 : %.c tcc_cc stdlib.c
	./tcc_cc -64 -o $@ stdlib.c $<

%_t64.sl64 : %.c tcc_cc.amd64 stdlib.c
	./tcc_cc.amd64 -64 -o $@ stdlib.c $< 

%.M1_amd64 : %.sl64 stack_c_amd64 stack_c_intro_amd64.M1
	./stack_c_amd64 -i stack_c_intro_amd64.M1 $< -o $@

%.blood_elf_amd64 : %.M1_amd64
	./blood-elf --64 --file $< --little-endian --output $@

%.macro_amd64 : %.blood_elf_amd64
	./M1 $(basename $@).M1_amd64 -o $@

%.amd64 : %.macro_amd64
	./hex2 -o $@ ../M2libc/amd64/ELF-amd64-debug.hex2 $(basename $@).macro_amd64 $(basename $@).blood_elf_amd64
	objdump -d -M intel $@ >$@.txt

%.amd64_hex0 : %.macro_amd64
	./hex2 -o $@ ../M2libc/amd64/ELF-amd64-debug.hex2 $(basename $@).macro_amd64 $(basename $@).blood_elf_amd64

unittest.amd64_OK : unittest.amd64 stack_c_interpreter
	./unittest.amd64
	./stack_c_interpreter unittest.sl64

stack_c_diff_amd64_OK : stack_c_amd64.amd64 stack_c_intro_amd64.M1 stack_c.sl64 stack_c.M1_amd64
	./stack_c_amd64.amd64 -i stack_c_intro_amd64.M1 stack_c.sl64 -o stack_c_s2.M1_amd64
	./run.sh diff stack_c.M1_amd64 stack_c_s2.M1_amd64 stack_c_diff_amd64_OK

tcc_cc_diff_amd64_OK : stack_c_amd64.amd64 stack_c_intro_amd64.M1 tcc_cc.sl64 tcc_cc tcc_cc.M1_amd64
	./stack_c_amd64.amd64 -i stack_c_intro_amd64.M1 tcc_cc.sl64 -o tcc_cc_s2.M1_amd64
	./run.sh diff tcc_cc.M1_amd64 tcc_cc_s2.M1_amd64 tcc_cc_diff_amd64_OK
	
diff3_amd64_OK : unittest.sl64 unittest_t64.sl64
	./run.sh diff unittest.sl64 unittest_t64.sl64 diff3_amd64_OK

diff4_amd64_OK : stack_c.sl64 stack_c_t64.sl64
	./run.sh diff stack_c.sl64 stack_c_t64.sl64 diff4_amd64_OK

diff5_amd64_OK : tcc_cc.sl64 tcc_cc_t64.sl64
	./run.sh diff tcc_cc.sl64 tcc_cc_t64.sl64 diff5_amd64_OK

.PRECIOUS: %.sl %_s.M1 %.blood_elf %.macro \
    %.sl64 %.M1_amd64 %.blood_elf_amd64 %.macro_amd64
.PHONY: all x86 amd64
