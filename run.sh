#!/bin/bash
if [ "$1" == "gcc" ]; then
  echo gcc $2.c
  gcc -g -Wall -Werror $2.c -o $2 2>errors.txt
  if [ $? -ne 0 ]; then
    clear
    head errors.txt
    exit 1
  fi
fi
if [ "$1" == "diff" ]; then
  diff $2 $3 >errors.txt
  if [ $? -eq 0 ]; then
    echo No differences stack_c.M1 stack_c2.M1
  else
  	head errors.txt
    echo Failed
    exit 1
  fi
fi
if [ "$1" == "tcc_cc2" ]; then
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
fi
