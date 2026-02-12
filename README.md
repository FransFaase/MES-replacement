# GNU Mes replacement

The goal of this project is to simplify stage0 of
[live-bootstrap](https://github.com/fosslinux/live-bootstrap),
which involves implementing a replacement for the
[GNU Mes compiler](https://www.gnu.org/software/mes/)
by implementing a C-compiler in C that can compile the
[Tiny C Compiler](https://en.wikipedia.org/wiki/Tiny_C_Compiler) version 0.9.26.

The motivation for this project is given in the presentation
[Reviewing live-bootstrap](https://www.iwriteiam.nl/WHY2025_talk.html).

For blog article related to reviewing the live bootstrap project and this
project see the section 'Live-bootstrap' on [this page](https://www.iwriteiam.nl/Software.html).

## Task 1: Compile the Tiny C Compiler correctly

The first stage of this project is to implement said C-compiler for i386.
The source of the [C-compiler](tcc_cc.md) is the file [`src/tcc_cc.c`](src/tcc_cc.c).
This compiler produces intermediate code for a stack based language
called [Stack-C](Stack_C.md). The compiler also includes the file
[`src/stdlib.c`](src/stdlib.c), that contains a minimal version of the
C standard library.

The intermediage code can be compiled with the program
[`src/stack_c.c`](src/stack_c.c) to M1 assembly or interpreted with the
program [`src/stack_c_interpreter.c`](src/stack_c_interpreter.c).

To verify the correct compilation, already some parts of Task 2 and 3 were
developed, because a change root environment needed to be used to get the
correct paths into the compiled executables.

The following Linux programs are required (most of which should already
be available in a default development environment): `git`, `wget`, `gcc`, `make`,
`diff`, `patch`, `chroot`, `tar`, and `gunzip`.

If you have not cloned the repository with the `--recurse-submodules` command line
option, issue the command:
```sh
git submodule update --init --recursive
```
This will retrieve the submodules `M2libc` and GNU Mes sources. To also retrieve
the correct Tiny C Compilers sources, issue the command:
```sh
./download_tcc.sh
```
Next build the necessary seeds and source files in the `src` directory with the
commands:
```sh
cd src
make
cd ..
```
Finally, give the following command, which will create a change root environment
in the directory `rootfs` and execute a `chroot` command to build the Tiny C Compiler in
that directory:
```sh
./task1.sh
```
The output should end with:
```
 +> /usr/bin/sha256sum -c /steps/tcc-0.9.26/tcc-0.9.26.x86.checksums 
/usr/bin/tcc-boot0: FAILED
Wanted:   25698c9689995cad9dcf3dd834526e7ef97fba27cef6367c0e618b9ad6c0657d
Received: 632799650bc185e815a863796a80b5f43199a2796ba9bd2872ff97ca91c3003f
/usr/bin/tcc-boot1: OK
/usr/bin/tcc-boot2: OK
/usr/lib/mes/libc.a: OK
/usr/lib/mes/libgetopt.a: OK
/usr/lib/mes/crt1.o: OK
/usr/lib/mes/crti.o: OK
/usr/lib/mes/crtn.o: OK
/usr/lib/mes/tcc/libtcc1.a: OK
Subprocess error 1
ABORTING HARD
```
This shows that the `rootfs/bin/usr/tcc-boot2` is equal to the `tcc-0.0.26` executable
build during the execution of live-bootstrap. That `tcc-boot0` is different can
probably be explained by the fact that the GNU Mes compiler has support of 64 bits integers
while `tcc_cc` lacks this.

## Task 2: Compile the required utilities

This task will focus on removing the dependency of the
executables from stage0 by building these with the C-compiler.
These also include compiling some utilities, such as ungz and untar,
from C sources that are needed, for example, to unpack the sources
of the Tiny C Compiler, such that they can be compiled.

The following utilities, taken from the live-bootstrap sources, have
been compiled successful (with small modifications and/or combining
sources files into a single file):
- `blood-elf` from [`blood-elf.c`](src/blood-elf.c)
- `catm` from [`catm.c`](src/catm.c)
- `chmod` from [`chmod.c`](src/chmod.c)
- `configurator` from [`configurator.c`](src/configurator.c)
- `cp` from [`cp.c`](src/cp.c)
- `kaem` from [`kaem.c`](src/kaem.c), which is original files merged into one.
- `match` from [`match.c`](src/match.c)
- `mkdir` from [`mkdir.c`](src/mkdir.c)
- `rm` from [`rm.c`](src/rm.c)
- `script-generator` from [`script-generator.c`](src/script-generator.c)
- `sha256sum` from [`sha256sum.c`](src/sha256sum.c)
- `unbz2` from [`unbz2.c`](src/unbz2.c)
- `ungz` from [`ungz.c`](src/ungz.c)
- `untar` from [`untar.c`](src/untar.c)
- `unxz` from [`unxz.c`](src/unxz.c)

Many of the above C sources also include [`bootstrappable.c`](src/bootstrappable.c)

## Task 3: New kaem scripts

Develop the kaem scripts for the new C compiler. This probably have
to be done in parallel with Task 2, because it is not clear which
utilities exactly will be needed.

**Remark**: The description below assumes that the steps mentioned
with Task 1, up to the execution of script `task1.sh`, have been
performed.

The shell script `task3.sh` first creates the `rootfs` directory with
additional subdirectories and files, such that it can be used as a
change root environment. Some source files are taken from the `src`
directory (assuming that `make` has been executed in that directory)
and some files and kaem scripts taken from the `task3` directory.
Next the `task3.sh` script uses the `chroot` command to execute the
kaem script in the `rootfs` change root environment. The output of
the `task3.sh` script should end with:
```
 +> tcc -version 
tcc version 0.9.27 (i386 Linux)
 +> if match xFalse xTrue 
/usr/bin/tcc: OK
 +> cd .. 
```
This shows that the `rootfs/usr/bin` directory contains a `tcc` executable,
which is the same as the `tcc` executable build by the live-bootstrap project
(in a change root environment) from tcc 0.9.27, which is build with the
`tcc-0.9.26` executable. That they are the same is based on the executables
having the same SHA256 hash. 

The kaem scripts are basically following the same structure as that
found in live-bootstrap. The `script-generator` is called on a version
of the `manifest` file that only contains the steps needed to compile
the tcc 0.9.27 sources.

For documentation, including a T-diagram, generated from the output of
the Linux `strace` command with the help of the
[`scan_trace.cpp`](scan_trace.cpp) program see
[fransfaase.github.io/MES-replacement/](https://fransfaase.github.io/MES-replacement/).

## Task 4: versions for hex0 and M1

For this task a number of C programs have been developed which are
compiled with the help op `tcc_cc`, (some using the `stdlib.c` file
as a replacement for the standard library), `stack_c`, `blood-elf`,
`M1` and `hex2`. Some of them are alternatives in order to generate
`hex0` files with comments that information about the assembly
instructions, the stack_c commands, and references to the C source
lines, in order to relationship explicit for review purposes. Others
are alternatives for which no C code was available or new programs.

For an example of a `hex0` file with references, see:
[`hex0.hex0`](https://fransfaase.github.io/MES-replacement/listing#F4).
The C source line number that can be found in there reference
[`hex0.c`](src/hex0.c). Take for example line 34 in this C program:
```c
       if (ch <= ' ')
```
The C compiler compiles this to the intermediate language (stack_c) into:
```
     ch ?1 32 <=s if {
```
This then is compiled (with `stack_c`, `M1` and `hex2`) in the following fragment
in `hex0.hex0`, which has a three column format, where the first shows the
hexadecimal representation matching the assembly instruction in the
second column, which are generated from the intermediage language shown
in the third column:
```
                             ## hex0.c 34
                             #:_main_else2 # no else
50                           #  push_eax              # ch (local)
8D85 1C000000                #  lea_eax,[ebp+DWORD] %28
8A00                         #  mov_al,[eax]          # ?1
0FB6C0                       #  movzx_eax,al
50                           #  push_eax              # 32
B8 20000000                  #  mov_eax, %32
5B                           #  pop_ebx               # <=s
39C3                         #  cmp_eax_ebx
0F9EC0                       #  setle_al
0FB6C0                       #  movzx_eax,al
85C0                         #  test_eax,eax          # if
58                           #  pop_eax
0F84 05000000                #  je %_main_else3
```
(The line with `_main_else2` is part of the previous statement.)

### hex0.c

[`hex0.c`](src/hex0.c) is a C program that is used to produce `hex0.hex0` and the
`hex0` seed. These are alternatives that a longer than those used in
live-bootstrap, but they have the advantage that they are compiled with
the tools and have comments refering to the original C source, such that
comparison is possible.

### hex2.c

[`hex2.c`](src/hex2.c) is a C program that is used to produce an alternative for
`hex2` that can produce both binary files as `hex0` files based on
the extension of the output file. For the `hex0` files, it retains
the input as comments.

### M1.c

[`M1.c`](src/M1.c) is a C program that is used to produce an alternative for `M1`
which does copy the input as comments. It only supports the 'operators'
needed for the x86 target. It might need to implement additional
'operators' for 32-bits targets.

### equal.c

[`equal.c`](src/equal.c) is a C program that is used to produce the `equal` program
that compares the two file with the names given as command line arguments.
It returns 0 is the files are equal, otherwise a non-zero value.

This program is used in the `check-tools.kaem` script to verify that
the various intermediate files placed in the `x86` and `scr` directories
van be compiled from the original C programs with the compiled command
from the compiler and assembly tools.

### kaem-minimal.c

[`kaem-minimal.c`](src/kaem-minimal.c) is a C program that is used to produce an alternative
for `kaem-minimal`, which is used to execute the initial `kaem` files.

## Task 5: Implement support for other targets

Write versions of the program converting the intermediate stack language
for other targets besides x86. For x86_64 this will be relatively simply.
For the other targets this requires some investigation, also figuring out
how to test this, for example, using some form of emulation, such as QEMU.

Targets:
* x86_64
* arm
* riscv64

## Task 6: Presentation and documentation

Write the necessary MarkDown/HTML files with the alternative git repository
for stage0 and add comments to the source files where needed. Write or
give a presentation about the achievements.

## Older files

For a first feasability study to analyzing what part of the C-grammar
that is needed for compiling TCC. For this I wrote a minimal C preprocessor:
`min_tcc_preprocessor.cpp` and `CParser.c` that is heavily based on
[RawParser](https://github.com/FransFaase/RawParser).

# Acknowledgments

The work in this repository falls under the project
[Verifying and documenting live-bootstrap](https://nlnet.nl/project/live-bootstrap/),
which was funded through the [NGI0 Core](https://nlnet.nl/core/) Fund, a fund established by
[NLnet](https://nlnet.nl) with financial support from the European Commission's
[Next Generation Internet](https://ngi.eu) programme, under the aegis of
[DG Communications Networks, Content and Technology](https://commission.europa.eu/about-european-commission/departments-and-executive-agencies/communications-networks-content-and-technology_en) under grant agreement
Nₒ [101092990](https://cordis.europa.eu/project/id/101092990).
