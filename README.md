# GNU Mes replacement

Investigation into replacing the [GNU Mes compiler](https://www.gnu.org/software/mes/)
which is part of [live-bootstrap](https://github.com/fosslinux/live-bootstrap).

The reason for this is that the GNU Mes compiler is a bit of odd one out because
it is compiled with a C-preprocessor and C-compiler and it is used to compile
Tiny C Compiler, which itself might only use a subset of C itself. The gap
between the two seems rather small.
