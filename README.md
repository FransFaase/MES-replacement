# GNU Mes replacement

Investigation into replacing the [GNU Mes compiler](https://www.gnu.org/software/mes/)
which is part of [live-bootstrap](https://github.com/fosslinux/live-bootstrap).

The reason for this is that the GNU Mes compiler is a bit of odd one out because
it is compiled with a C-preprocessor and C-compiler and it is used to compile
Tiny C Compiler, which itself might only use a subset of C itself. The gap
between the two seems rather small.

## Analyzing C-grammar used for TCC

The first step consists of analyzing what part of the C-grammar that is needed
for compiling TCC. For this I wrote a minimal C preprocessor: `min_tcc_preprocessor.cpp`
and `CParser.c` that is heavily based on [RawParser](https://github.com/FransFaase/RawParser).

## WIP 11 July 2024

The [commit 058272](https://github.com/FransFaase/MES-replacemen/commit/4e31a615bcc408b6351247f035f348935121d26f)
adds the files `tcc_cc.c` and `gcc_tcc_cc.c`. The second includes the first. The first is
an attempt to implement an iterator based C-preprocessor (which is not finished yet). It
does work when compiling `gcc_tcc_cc.c` with the following command:
```
gcc -Wno-builtin-declaration-mismatch -Wno-int-conversion -g gcc_tcc_cc.c -o gcc_tcc_cc
```
But when I compile it with `M2-Mesoplanet` using the following command:
```
M2-Mesoplanet -f tcc_cc.c --architecture x86 --max-string 6000 -o tcc_cc
```
it returns a segmentation fault after some time in a place where before it did not give it.
It looks like the return address on the stack is overwritten at some point. The error
also occurs when using [`Emulator`](https://github.com/FransFaase/Emulator/).
