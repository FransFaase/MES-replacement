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

# Task 1: Compile the Tiny C Compiler correctly

The first stage of this project is to implement said C-compiler for i386.
The source of the [C-compiler](tcc_cc.md) is the file [`tcc_cc.c`](tcc_cc.c).
This compiler produces intermediate code for a stack based language
called [Stack-C](Stack_C.md). The compiler also includes the file
[`stdlib.c`](stdlib.c), that contains a minimal version of the
C standard library.

The intermediage code can be compiled with the program
[`stack_c.c`](stack_c.c) to M1 assembly or interpreted with the
program [`stack_c_interpreter.c`](stack_c_interpreter.c).

This stage depends on a number of executables from stage0. Namely:
- hex2
- M1
- blood-elf
- catm
- match
- sha256sum

These need to be present in the directory of the repository.

Furthermore it requires the usual Linux commands and the GNU C compiler.
[A makefile](Makefile) is included to build and test the
C-compiler `tcc_cc`.

The sources of the Tiny C Compiler should be placed in a directory
with the name `tcc_sources` that should also have sub directory
`lib`.

There should also be a directory `mes` with the contents of the
GNU Mes compiler, which is needed to build the standard library
that the Tiny C Compiler needs.

To build and test the Tiny C Compiler, the [`test.sh`](test.sh)
shell script is provided. This script first compiles the Tiny C
Compiler with GNU C-compiler, resulting in `tcc_g` and with
tcc_cc compiler, resulting in `tcc_s`. Next is uses these to
bootstrap the Tiny C Compiler from the sources. The script
compares the results for the various steps using `tcc_g` and
`tcc_c`.

Remark: The `test.sh` script assumes that this repository is cloned
in the `git` directory in the home directory. Please update the
`BINDIR` to point to the repository containing the repository.

This stage has been implemented.

## Task 2: Compile the required utilities

This task will focus on removing the dependency of the
executables from stage0 by building these with the C-compiler.
These also include compiling some utilities, such as ungz and untar,
from C sources that are needed, for example, to unpack the sources
of the Tiny C Compiler, such that they can be compiled.

## Task 3: New kaem scripts

Develop the kaem scripts for the new C compiler. This probably have
to be done in parallel with Task 2, because it is not clear which
utilities exactly will be needed.

## Task 4: versions for hex0 and M1

Write a C version for M1 that retains the comments to facilitate
the review of the generated intermediate files. Write a C version
for hex0, that might be a bit longer than the current 'minimal' hex0,
but might be more easily to review.

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
