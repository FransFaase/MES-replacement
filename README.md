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
Nₒ [101092990](https://cordis.europa.eu/project/id/101092990).
