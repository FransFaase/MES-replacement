#!/bin/bash
exetime=`stat -c %Y tcc_cc`
mdtime=`stat -c %Y tcc_cc.c`
if [ -z "${exetime}" ] || [ $mdtime -gt $exetime ]; then
  echo gcc tcc_cc.c
  gcc -g -Wall -Werror tcc_cc.c -o tcc_cc 2>errors.txt
  if [ $? -ne 0 ]; then
    clear
    head errors.txt
    exit 1
  fi
fi
exetime=`stat -c %Y stack_c`
mdtime=`stat -c %Y stack_c.c`
if [ -z "${exetime}" ] || [ $mdtime -gt $exetime ]; then
  echo gcc stack_c
  gcc -g -Wall -Werror stack_c.c -o stack_c 2>errors.txt
  if [ $? -ne 0 ]; then
    clear
    head errors.txt
    exit 1
  fi
fi
echo tcc_cc unittest.c
./tcc_cc unittest.c
if [ $? -ne 0 ]; then
  exit 1
fi
echo stack_c unittest
./stack_c output.sl >unittest.M1
echo blood-elf unittest
./blood-elf --file unittest.M1 --little-endian --output unittest.blood_elf
echo M1 unittest
./M1 --file unittest.M1 --little-endian --architecture x86 --file unittest.blood_elf --output unittest.macro
echo hex2 unittest
./hex2 --file ./M2libc/x86/ELF-x86-debug.hex2 --file unittest.macro --output unittest --architecture x86 --base-address 0x8048000 --little-endian 
echo objdump unittest
objdump -d unittest >unittest.txt
echo unittest
./unittest >unittest_out.txt
if [ $? -ne 0 ]; then
  exit 1
fi
echo tcc_cc stack_c.c
./tcc_cc stack_c.c
if [ $? -ne 0 ]; then
  exit 1
fi
echo stack_c stack_c
./stack_c output.sl >stack_c.M1
echo blood-elf stack_c
./blood-elf --file stack_c.M1 --little-endian --output stack_c.blood_elf
echo M1 stack_c
./M1 --file stack_c.M1 --little-endian --architecture x86 --file stack_c.blood_elf --output stack_c.macro
echo hex2 stack_c
./hex2 --file ./M2libc/x86/ELF-x86-debug.hex2 --file stack_c.macro --output stack_c2 --architecture x86 --base-address 0x8048000 --little-endian 
echo objdump stack_c2
objdump -d stack_c2 >stack_c2.txt
echo stack_c2
./stack_c2 output.sl >stack_c2.M1
#if [ $? -ne 0 ]; then
#  echo Failed
#  exit 1
#fi
diff stack_c.M1 stack_c2.M1
if [ $? -eq 0 ]; then
  echo No differences stack_c.M1 stack_c2.M1
else
  echo Failed
  exit 1
fi

echo tcc_cc tcc_cc.c
./tcc_cc tcc_cc.c
if [ $? -ne 0 ]; then
  exit 1
fi
echo stack_c tcc_cc
./stack_c output.sl >tcc_cc.M1
if [ $? -ne 0 ]; then
  exit 1
fi
echo blood-elf tcc_cc
./blood-elf --file tcc_cc.M1 --little-endian --output tcc_cc.blood_elf
echo M1 tcc_cc
./M1 --file tcc_cc.M1 --little-endian --architecture x86 --file tcc_cc.blood_elf --output tcc_cc.macro
echo hex2 tcc_cc
./hex2 --file ./M2libc/x86/ELF-x86-debug.hex2 --file tcc_cc.macro --output tcc_cc2 --architecture x86 --base-address 0x8048000 --little-endian 
echo objdump tcc_cc2
objdump -d tcc_cc2 >tcc_cc2.txt
echo tcc_cc2
cp output.sl output_org.sl
./tcc_cc2 tcc_cc
#if [ $? -ne 0 ]; then
#  echo Failed
#  exit 1
#fi
diff output_org.sl output.sl
if [ $? -eq 0 ]; then
  echo No differences output_org.sl output.sl
else
  echo Failed
  exit 1
fi
echo Done!
