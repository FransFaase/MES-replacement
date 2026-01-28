#!/bin/sh
clear
set -x

src/hex0_s src/hex0_s.hex0 bin/hex0

bin/hex0 src/hex2_s.hex0 bin/hex2

bin/hex2 -o bin/blood-elf M2libc/x86/ELF-x86-debug.hex2 src/blood-elf_s.macro src/blood-elf_s.blood_elf

bin/hex2 -o bin/M1 M2libc/x86/ELF-x86-debug.hex2 src/M1_s.macro src/M1_s.blood_elf

bin/blood-elf --file src/stack_c_s.M1 --little-endian --output tmp/stack_c_s.blood_elf
bin/M1 src/stack_c_s.M1 -o tmp/stack_c_s.macro
bin/hex2 M2libc/x86/ELF-x86-debug.hex2 tmp/stack_c_s.macro tmp/stack_c_s.blood_elf -o bin/stack_c

bin/stack_c -i src/stack_c_intro.M1 src/tcc_cc.sl -o tmp/tcc_cc.M1
bin/blood-elf --file tmp/tcc_cc.M1 --little-endian --output tmp/tcc_cc.blood_elf
bin/M1 tmp/tcc_cc.M1 -o tmp/tcc_cc.macro
bin/hex2 M2libc/x86/ELF-x86-debug.hex2 tmp/tcc_cc.macro tmp/tcc_cc.blood_elf -o bin/tcc_cc

echo Done
#echo gcc
#gcc -g -Wall -Werror hex2.c -o my_hex2
#echo tcc_cc
#./tcc_cc -o hex0.sl hex0.c
#echo stack_c
#./stack_c hex0.sl -o hex0.M1
#echo blood-elf
#./blood-elf --file hex0.M1 --little-endian --output hex0.blood_elf
#echo M1
#./M1 --file hex0.M1 --little-endian --architecture x86 --file hex0.blood_elf --output hex0.macro
#echo hex2
#./hex2 --file ./M2libc/x86/ELF-x86-debug.hex2 --file hex0.macro --output hex0 --architecture x86 --base-address 0x8048000 --little-endian
#echo my_M1
#./my_M1 hex0.M1 my_hex0.macro
#echo my_hex2
#./my_hex2 -o my_hex0.hex0 M2libc/x86/ELF-x86-debug.hex2 my_hex0.macro hex0.blood_elf
#./my_hex2 -o my_hex0 M2libc/x86/ELF-x86-debug.hex2 my_hex0.macro hex0.blood_elf
#diff hex0 my_hex0
#./my_hex0 my_hex0.hex0 my_hex0_2
#diff hex0 my_hex0_2
#echo OK

