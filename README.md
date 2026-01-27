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

This stage depends on a number of executables from stage0. Namely:
- hex2
- M1
- blood-elf
- catm
- match
- sha256sum

These need to be present in the directory `org_stage0`.

Furthermore it requires the usual Linux commands and the GNU C compiler.
[A makefile](src\Makefile) is included in the `src` directory to build
and test the C-compiler `tcc_cc`.

The sources of the Tiny C Compiler should be placed in a directory
with the name `tcc_sources` that should also have sub directory
`lib`.

There should also be a directory `mes` with the contents of the
GNU Mes compiler 0.27.1, which is needed to build the standard library
that the Tiny C Compiler needs. It is sufficient to extract the
directories `lib` and `include` from `mes-0.27.1.tar.gz`.

To build and test the Tiny C Compiler, the [`task1/test.sh`](task1/test.sh)
shell script is provided. This script first compiles the Tiny C
Compiler with GNU C-compiler, resulting in `tcc_g` and with
tcc_cc compiler, resulting in `tcc_s`. Next is uses these to
bootstrap the Tiny C Compiler from the sources. The script
compares the results for the various steps using `tcc_g` and
`tcc_c`.

This stage has been implemented and it has been verified that
although the resulting `tcc` executable is not the same as the
one produced with the live-bootstrap project, it seems that the
differences are only due to some global variables being a bit
larger. The cause of this has not been established, but it could
be due to the fact that it is compiled in a different environment.
The program [`asmdiff.c`](task1/asmdiff.c) has been developed for
verifying this. It seems that only instructions that move data
from and to fixed memory locations are used. The program outputs
the regions that have been offset and verifies that they do not overlap.

## Task 2: Compile the required utilities

This task will focus on removing the dependency of the
executables from stage0 by building these with the C-compiler.
These also include compiling some utilities, such as ungz and untar,
from C sources that are needed, for example, to unpack the sources
of the Tiny C Compiler, such that they can be compiled.

The following utilities have been compiled successful:
- `kaem-minimal`
- `kaem`
- `catm`

## Task 3: New kaem scripts

Develop the kaem scripts for the new C compiler. This probably have
to be done in parallel with Task 2, because it is not clear which
utilities exactly will be needed.

The shell script `task3_init.sh` creates the `rootfs` directory with
additional subdirectories and files, such that the script `task3.sh`
can execute it and use it as a change root environment. In the `task3`
directory the various kaem scripts are found that are executed.
After execution the `rootfs/usr/bin` directory contains the `tcc-boot2`
executable, which is the same as the `tcc` executable build by the
live-bootstrap project (in a change root environment).

## Task 4: versions for hex0 and M1

Write a C version for M1 that retains the comments to facilitate
the review of the generated intermediate files. Write a C version
for hex0, that might be a bit longer than the current 'minimal' hex0,
but might be more easily to review.

C files for alternatives for `hex0`, `hex2`, `M1`, and `blood-elf`
can be found in the `src` directory. The sources for `bloof-elf` are
based on those from live-bootstrap with some small modifications such
that they can be compiled with `tcc_cc`. The others are new with some
modifications. When `hex2` is called to output a file with the `hex0`
extension it produces a `hex0` file with comments based on the output
of `stack_c` containing references to the C sources.

## Task 5: Implement support for other targets

Write versions of the program converting the intermediate stack language
for other targets besides x86. For x86_64 this will be relatively simply.
For the other targets this requires some investigation, also figuring out
how to test this, for example, using some form of emulation, such as QEMU.

Targets:
* x86_64
* arm
* riscv64

## Taks 6: Presentation and documentation

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
Nₒ [101092990](https://cordis.europa.eu/project/id/101092990").
