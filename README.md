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

# Stage 1

The first stage of this project is to implement said C-compiler for i386.
The source of the C-compiler is the file `tcc_cc.c`. This
compiler produces intermediate code in a stack based language.
The intermediage code can be compiled with the program `stack_c.c`
to assembly or interpretted with the program `stack_c_interpreter.c`

This stage depends on a number of executables from stage0. Namely:
- hex2
- M1
- blood-elf
- catm
- match
- sha256sum
These need to be present in the directory of the repository.

Furthermore it requires the usual Linux commands and the GNU C compiler.
A makefile is included to build and test the C-compiler `tcc_cc`.

The sources of the Tiny C Compiler should be placed in a directory
with the name `tcc_sources` that should also have sub directory
`lib`.

There should also be a directory `mes` with the contents of the
GNU Mes compiler, which is needed to build the standard library
that the Tiny C Compiler needs.

To build and test the Tiny C Compiler, the `test.sh` shell script
is provided. This script first compiles the Tiny C Compiler with
GNU C-compiler, resulting in `tcc_g` and with tcc_cc compiler,
resulting in `tcc_s`. Next is uses these to bootstrap the Tiny
C Compiler from the sources. The script compares the results
for the various steps using `tcc_g` and `tcc_c`.

Remark: The `test.sh` script assumes that this repository is cloned
in the `git` directory in the home directory. Please update the
`BINDIR` to point to the repository containing the repository.

## Stage 2

The second stage will focus on removing the dependency of the
executables from stage0 by building these with the C-compiler.

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
