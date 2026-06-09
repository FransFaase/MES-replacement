all : x86 amd64
x86 : tools stack_c unittest.x86_OK stack_c.x86 stack_c_diff_OK tcc_cc.x86 tcc_cc_diff_OK diff3_OK diff5_OK diff4_OK \
      hex0.x86 hex0.x86_hex0 hex2.x86_hex0 \
	  blood-elf.macro_x86 blood-elf.blood_elf_x86 \
	  M1.macro_x86 M1.blood_elf_x86 \
	  stack_c.M1_x86 tcc_cc.sl \
	  kaem-minimal.x86 kaem-minimal.x86_hex0 \
	  kaem.x86 \
	  catm.x86 \
	  match.x86 \
	  mkdir.x86 cp.x86 chmod.x86 rm.x86 \
	  untar.x86 ungz.x86 unxz.x86 unbz2.x86 \
	  configurator.x86 script-generator.x86 \
      sha256sum.x86 \
	  equal.x86

tools : tcc_cc M1 hex2 hex0 blood-elf

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

%_t.sl : %.c tcc_cc.x86 stdlib.c
	./tcc_cc.x86 -o $@ stdlib.c $<

%.M1_x86 : %.sl stack_c stack_c_intro.M1
	./stack_c -i stack_c_intro.M1 $< -o $@

%.blood_elf_x86 : %.M1_x86 blood-elf
	./blood-elf --file $< --little-endian --output $@

%.macro_x86 : %.blood_elf_x86 M1
	./M1 $(basename $@).M1_x86 -o $@

%.x86 : %.macro_x86
	./hex2 -o $@ ../M2libc/x86/ELF-x86-debug.hex2 $(basename $@).macro_x86 $(basename $@).blood_elf_x86
	objdump -d -M intel $@ >$@.txt

%.x86_hex0 : %.macro_x86
	./hex2 -o $@ ../M2libc/x86/ELF-x86-debug.hex2 $(basename $@).macro_x86 $(basename $@).blood_elf_x86

unittest.x86_OK : unittest.x86 stack_c_interpreter unittest.sl
	./unittest.x86
	./stack_c_interpreter unittest.sl

stack_c_diff_OK : stack_c.x86 stack_c_intro.M1 stack_c.sl stack_c.M1_x86
	./stack_c.x86 -i stack_c_intro.M1 stack_c.sl -o stack_c_s2.M1_x86
	./run.sh diff stack_c.M1_x86 stack_c_s2.M1_x86 stack_c_diff_OK

tcc_cc_diff_OK : stack_c.x86 stack_c_intro.M1 tcc_cc.sl tcc_cc.M1_x86
	./stack_c.x86 -i stack_c_intro.M1 tcc_cc.sl -o tcc_cc_s2.M1_x86
	./run.sh diff tcc_cc.M1_x86 tcc_cc_s2.M1_x86 tcc_cc_diff_OK
	
diff3_OK : unittest.sl unittest_t.sl
	./run.sh diff unittest.sl unittest_t.sl diff3_OK

diff4_OK : stack_c.sl stack_c_t.sl
	./run.sh diff stack_c.sl stack_c_t.sl diff4_OK

diff5_OK : tcc_cc.sl tcc_cc_t.sl
	./run.sh diff tcc_cc.sl tcc_cc_t.sl diff5_OK

amd64 : stack_c_amd64 unittest.amd64_OK stack_c_diff_amd64_OK tcc_cc_diff_amd64_OK diff3_amd64_OK diff5_amd64_OK diff4_amd64_OK \
      hex0.amd64 hex0.amd64_hex0 hex2.amd64_hex0 \
	  blood-elf.macro_amd64 blood-elf.blood_elf_amd64 \
	  M1.macro_amd64 M1.blood_elf_amd64 \
	  stack_c_amd64.M1_amd64 tcc_cc.amd64.sl64 \
	  kaem-minimal.amd64 kaem-minimal.amd64_hex0 \
	  kaem.amd64 \
	  catm.amd64 \
	  match.amd64 \
	  mkdir.amd64 cp.amd64 chmod.amd64 rm.amd64 \
	  untar.amd64 ungz.amd64 unxz.amd64 unbz2.amd64 \
	  configurator.amd64 script-generator.amd64 \
      sha256sum.amd64 \
	  equal.amd64

hex0.amd64.sl64 : hex0.c tcc_cc
	./tcc_cc -64 -o $@ $<

hex2.amd64.sl64 : hex2.c tcc_cc
	./tcc_cc -64 -o $@ $<

kaem-minimal.amd64.sl64 : kaem-minimal.c tcc_cc
	./tcc_cc -64 -o $@ $<

equal.amd64.sl64 : equal.c tcc_cc
	./tcc_cc -64 -o $@ $<

%.amd64.sl64 : %.c tcc_cc stdlib.c
	./tcc_cc -64 -o $@ stdlib.c $<

%_tamd64.amd64.sl64 : %.c tcc_cc.amd64 stdlib.c
	./tcc_cc.amd64 -64 -o $@ stdlib.c $< 

%.M1_amd64 : %.amd64.sl64 stack_c_amd64 stack_c_intro_amd64.M1
	./stack_c_amd64 -i stack_c_intro_amd64.M1 $< -o $@

%.blood_elf_amd64 : %.M1_amd64 blood-elf
	./blood-elf --64 --file $< --little-endian --output $@

%.macro_amd64 : %.blood_elf_amd64 M1
	./M1 $(basename $@).M1_amd64 -o $@

%.amd64 : %.macro_amd64 hex2
	./hex2 -o $@ ../M2libc/amd64/ELF-amd64-debug.hex2 $(basename $@).macro_amd64 $(basename $@).blood_elf_amd64
	objdump -d -M intel $@ >$@.txt

%.amd64_hex0 : %.macro_amd64 hex2
	./hex2 -o $@ ../M2libc/amd64/ELF-amd64-debug.hex2 $(basename $@).macro_amd64 $(basename $@).blood_elf_amd64

unittest.amd64_OK : unittest.amd64 stack_c_interpreter
	./unittest.amd64
	./stack_c_interpreter unittest.amd64.sl64

stack_c_diff_amd64_OK : stack_c_amd64.amd64 stack_c_intro_amd64.M1 stack_c.amd64.sl64 stack_c.M1_amd64
	./stack_c_amd64.amd64 -i stack_c_intro_amd64.M1 stack_c.amd64.sl64 -o stack_c_s2.M1_amd64
	./run.sh diff stack_c.M1_amd64 stack_c_s2.M1_amd64 stack_c_diff_amd64_OK

tcc_cc_diff_amd64_OK : stack_c_amd64.amd64 stack_c_intro_amd64.M1 tcc_cc.amd64.sl64 tcc_cc.M1_amd64
	./stack_c_amd64.amd64 -i stack_c_intro_amd64.M1 tcc_cc.amd64.sl64 -o tcc_cc_s2.M1_amd64
	./run.sh diff tcc_cc.M1_amd64 tcc_cc_s2.M1_amd64 tcc_cc_diff_amd64_OK
	
diff3_amd64_OK : unittest.amd64.sl64 unittest_tamd64.amd64.sl64
	./run.sh diff unittest.amd64.sl64 unittest_tamd64.amd64.sl64 diff3_amd64_OK

diff4_amd64_OK : stack_c.amd64.sl64 stack_c_tamd64.amd64.sl64
	./run.sh diff stack_c.amd64.sl64 stack_c_tamd64.amd64.sl64 diff4_amd64_OK

diff5_amd64_OK : tcc_cc.amd64.sl64 tcc_cc_tamd64.amd64.sl64
	./run.sh diff tcc_cc.amd64.sl64 tcc_cc_tamd64.amd64.sl64 diff5_amd64_OK

# aarch64 (arm64). Fully-aarch64 seed artifacts: the cross tools (tcc_cc, stack_c_arm64,
# blood-elf, M1, hex2 — gcc-built, native to the build host) emit aarch64 output. Only the
# artifacts below are aarch64; they are reassembled into a working toolchain inside the
# arm64 chroot (see task_arm64.sh / target_arm64). `make arm64` is build-only — running an
# aarch64 binary on an x86_64 host needs qemu, so the self-test diffs are deliberately omitted.
arm64 : tools stack_c_arm64 \
	  hex0.arm64 hex0.arm64_hex0 hex2.arm64_hex0 \
	  blood-elf.macro_arm64 blood-elf.blood_elf_arm64 \
	  M1.macro_arm64 M1.blood_elf_arm64 \
	  stack_c_arm64.M1_arm64 tcc_cc.arm64.sl64 \
	  kaem-minimal.arm64 kaem-minimal.arm64_hex0 \
	  kaem.arm64 \
	  catm.arm64 \
	  match.arm64 \
	  mkdir.arm64 cp.arm64 chmod.arm64 rm.arm64 \
	  untar.arm64 ungz.arm64 unxz.arm64 unbz2.arm64 \
	  configurator.arm64 script-generator.arm64 \
      sha256sum.arm64 \
	  equal.arm64

# For building and running tests, use 'make arm64_tests'
arm64_tests : arm64 unittest.arm64_OK stack_c_diff_arm64_OK tcc_cc_s tcc_cc_diff_arm64_OK diff3_arm64_OK diff5_arm64_OK diff4_arm64_OK

# Self-contained tools (no stdlib.c); -D TCC_TARGET_ARM64=1 selects the aarch64 syscall
# numbers in sys_syscall.h and the *at wrappers these tools define for arm64.
hex0.arm64.sl64 : hex0.c tcc_cc
	./tcc_cc -a arm64 -o $@ $<

hex2.arm64.sl64 : hex2.c tcc_cc
	./tcc_cc -a arm64 -o $@ $<

kaem-minimal.arm64.sl64 : kaem-minimal.c tcc_cc
	./tcc_cc -a arm64 -o $@ $<

equal.arm64.sl64 : equal.c tcc_cc
	./tcc_cc -a arm64 -o $@ $<

%.arm64.sl64 : %.c tcc_cc stdlib.c
	./tcc_cc -a arm64 -o $@ stdlib.c $<

%_tarm64.arm64.sl64 : %.c tcc_cc.arm64 stdlib.c
	./tcc_cc.arm64 -a arm64 -o $@ stdlib.c $< 

%.M1_arm64 : %.arm64.sl64 stack_c_arm64 stack_c_intro_arm64.M1
	./stack_c_arm64 -i stack_c_intro_arm64.M1 $< -o $@

%.blood_elf_arm64 : %.M1_arm64 blood-elf
	./blood-elf --64 --file $< --little-endian --output $@

%.macro_arm64 : %.blood_elf_arm64 M1
	./M1 $(basename $@).M1_arm64 -o $@

# No objdump: the host has no aarch64 objdump (and we don't require one).
%.arm64 : %.macro_arm64 hex2
	./hex2 -o $@ ../M2libc/aarch64/ELF-aarch64-debug.hex2 $(basename $@).macro_arm64 $(basename $@).blood_elf_arm64

%.arm64_hex0 : %.macro_arm64 hex2
	./hex2 -o $@ ../M2libc/aarch64/ELF-aarch64-debug.hex2 $(basename $@).macro_arm64 $(basename $@).blood_elf_arm64

unittest.arm64_OK : unittest.arm64 stack_c_interpreter
	./unittest.arm64
	#./stack_c_interpreter unittest.arm64.sl64

stack_c_diff_arm64_OK : stack_c_arm64.arm64 stack_c_intro_arm64.M1 stack_c.arm64.sl64 stack_c.M1_arm64
	./stack_c_arm64.arm64 -i stack_c_intro_arm64.M1 stack_c.arm64.sl64 -o stack_c_s2.M1_arm64
	./run.sh diff stack_c.M1_arm64 stack_c_s2.M1_arm64 stack_c_diff_arm64_OK

tcc_cc_diff_arm64_OK : stack_c_arm64.arm64 stack_c_intro_arm64.M1 tcc_cc.arm64.sl64 tcc_cc.M1_arm64
	./stack_c_arm64.arm64 -i stack_c_intro_arm64.M1 tcc_cc.arm64.sl64 -o tcc_cc_s2.M1_arm64
	./run.sh diff tcc_cc.M1_arm64 tcc_cc_s2.M1_arm64 tcc_cc_diff_arm64_OK
	
diff3_arm64_OK : unittest.arm64.sl64 unittest_tarm64.arm64.sl64
	./run.sh diff unittest.arm64.sl64 unittest_tarm64.arm64.sl64 diff3_arm64_OK

diff4_arm64_OK : stack_c.arm64.sl64 stack_c_tarm64.arm64.sl64
	./run.sh diff stack_c.arm64.sl64 stack_c_tarm64.arm64.sl64 diff4_arm64_OK

diff5_arm64_OK : tcc_cc.arm64.sl64 tcc_cc_tarm64.arm64.sl64
	./run.sh diff tcc_cc.arm64.sl64 tcc_cc_tarm64.arm64.sl64 diff5_arm64_OK

.PRECIOUS: %.sl %.M1_x86 %.blood_elf_x86 %.macro_x86 \
	%.sl64 \
	%.M1_amd64 %.blood_elf_amd64 %.macro_amd64 \
	%.M1_arm64 %.blood_elf_arm64 %.macro_arm64
.PHONY: all tools x86 amd64 arm64 arm64_tests
