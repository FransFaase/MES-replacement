#!/bin/bash
#echo compile stack_c.cpp
#g++ -g stack_c.cpp -o stack_c
echo stack_c
./stack_c $1 >test_c.M1
echo blood-elf
./blood-elf --file test_c.M1 --little-endian --output test_c.blood_elf
echo M1
./M1 --file test_c.M1 --little-endian --architecture x86 --file test_c.blood_elf --output test_c.macro
echo hex2
./hex2 --file ./M2libc/x86/ELF-x86-debug.hex2 --file test_c.macro --output testsl_c --architecture x86 --base-address 0x8048000 --little-endian 
echo objdump
objdump -d testsl_c >testsl_c.txt
echo testsl
./testsl_c
echo Done!

# starting M2-Planet build
#M2-Planet --file /tmp/M2-Mesoplanet-000000 --output /tmp/M2-Planet-000000 --architecture x86 --debug 
# starting Blood-elf stub generation
#blood-elf --file /tmp/M2-Planet-000000 --little-endian --output /tmp/blood-elf-000000 
# starting M1 assembly
#M1 --file ./M2libc/x86/x86_defs.M1 --file ./M2libc/x86/libc-core.M1 --file /tmp/M2-Planet-000000 --little-endian --architecture x86 --file /tmp/blood-elf-000000 --output /tmp/M1-macro-000000 
# starting hex2 linking
#hex2 --file ./M2libc/x86/ELF-x86-debug.hex2 --file /tmp/M1-macro-000000 --output testcall --architecture x86 --base-address 0x8048000 --little-endian 
