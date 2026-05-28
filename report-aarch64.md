# arm64 (aarch64) TCC bootstrap — Milestones 1–3

This documents the aarch64 port of the seed bootstrap. **Milestone 1** is the
self-hosted seed compiler `tcc_s` for arm64, runnable under qemu-user on an
x86_64 host. **Milestone 2** is `tcc_s` building an arm64 `libc.a` and compiling
a static `hello` (including a variadic `printf`) that runs under qemu.
**Milestone 3** drives the boot0/1/2 loop to a byte-identical fixpoint.

The project's end goal is a live-bootstrap-style build *from nothing* inside a
chroot. `build_arm64.sh` is deliberately not that: it is a development harness
that runs outside any chroot, which is the simplest way to iterate and is needed
regardless, since qemu-user is supplied by the host. What it does keep from the
bootstrap discipline is that `tcc_s` itself uses no libc and no gcc in its
codegen path.

```
$ qemu-aarch64-static build/usr/bin/tcc_s -version
tcc version 0.9.26 (AArch64 Linux)

$ qemu-aarch64-static build/tmp/hello
Hello, World!
Variadic: answer = 42
```

`build_arm64.sh` runs all three milestones end to end.

## Prerequisites

- x86_64 Linux host with `gcc`, `make`, `tar`, and `qemu-aarch64-static`
  (Debian/Ubuntu: `apt install qemu-user-static`).
- Submodules initialized and the tcc tarball present:

```sh
git submodule update --init --recursive
ls distfiles/tcc-0.9.26.tar.gz
```

`tcc_s` is statically linked and self-contained, so qemu-user needs no aarch64
sysroot or glibc.

## Run

```sh
./build_arm64.sh
```

The script builds the seed tools with gcc, extracts the tcc source, applies the
two upstream `simple-patches`, then runs the seed pipeline to produce
`build/usr/bin/tcc_s` and executes `qemu-aarch64-static tcc_s -version`. Takes a
few seconds; it recreates `build/` on each run.

## The pipeline

The seed tools are native x86_64; only the final `tcc_s` is arm64 (run via qemu):

```
tcc.c --(tcc_cc -a arm64)--> .sl --(stack_c_arm64)--> .M1 --(blood-elf --64)--> .blood_elf
                                                          \--(M1)--> .macro --(hex2)--> tcc_s
```

- `tcc_cc -a arm64` emits arch-neutral 64-bit Stack-C (the `-a` flag only
  toggles 64-bit; the same `.sl` would serve amd64).
- `stack_c_arm64` translates Stack-C to aarch64 M1 assembly.
- `M1` + `hex2` assemble it into a static ELF, using
  `M2libc/aarch64/ELF-aarch64-debug.hex2` for the ELF header.

## What was added

- **`src/stack_c_intro_arm64.M1`** — self-contained arm64 runtime prologue.
  Register model: `x18` = operand stack pointer (`str x,[x18,-8]!` / `ldr
  x,[x18],8`), `x17` = locals base (BP, `[x17]` holds the frame return address),
  `x0` = top-of-stack accumulator, `x1` = second operand, `x16` = call target.
  Provides `_start` (brk-based bump allocator for the locals stack, argc/argv
  setup, `exit`), plus the `sys_syscall` and `sys_malloc` runtime functions, and
  every instruction-macro the backend emits. All hex is uppercase (this repo's
  `hex2` only accepts uppercase A–F).
- **`src/stack_c_arm64.c`** — Stack-C → aarch64 backend, ported emit-site by
  emit-site from `stack_c_amd64.c` (parsing/scoping logic is shared and
  unchanged). Conventions:
  - Calls: `blr x16`; the callee prologue stores `x30` to `[x17]`; `return`
    reloads `x30` from `[x17]` and `ret`. `()` adjusts the BP (`x17`) by
    `8*pos` around the call.
  - Immediates and addresses load via a PC-relative literal:
    `ldr w,#8 ; b #8 ; <4-byte literal>` (zero-extended).
  - Jumps load `&label` into `w16` and `br x16`; conditional jumps are an
    inverted fixed-offset skip (`b.<inv> #20`) over that load+br block.
  - Comparisons use `cmp` + `cset w0,<cond>`.
- **`build_arm64.sh`** — the seed-only build + qemu verification described above.
- **`src/sys_syscall.h`** — added an aarch64 syscall-number block (write=64,
  read=63, close=57, exit=93, …) selected by `TCC_TARGET_ARM64`. Inert for the
  existing x86/amd64 builds. Note: the file-related calls that x86 exposes as
  `open`/`access`/`mkdir`/… only exist as `*at` variants on aarch64; the numbers
  are the `*at` syscalls, so those wrappers still need adapting (Milestone 2).
  `write`/`read`/`close`/`exit` — all that `tcc_s -version` needs — are correct.

## Verification approach

Every aarch64 instruction encoding was checked against `llvm-mc --triple=aarch64
--show-encoding`. The chain was validated incrementally under qemu with small
programs (exit codes and stdout) before compiling `tcc.c`: nested function calls
with arguments, arithmetic, `for` loops, `if/else`, comparisons, string output
via `write`, and variadic `printf`.

## Milestone 2 — arm64 libc.a

`build_arm64.sh` continues past `tcc_s`: it extracts the mes tarball, overlays
the aarch64 libc sources/headers from the `mes/` submodule, then drives `tcc_s`
(under qemu) to build `crt1.o`, `libc.a`, `libtcc1.a`, and `libgetopt.a`, and
finally compiles+runs a static `hello.c`.

### The central blocker: tcc 0.9.26 has no arm64 assembler

`arm64_FILES` is only `arm64-gen.c arm64-link.c` — there is no `arm64-asm.c`, and
`arm64-gen.c` never `#define`s `CONFIG_TCC_ASM`, so any `asm()` is
`tcc_error("inline asm() not supported")`. Every mes libc arch primitive
(`_start`, `__sys_call*`, `setjmp`) is inline asm, so none could be compiled.

- **`src/arm64-asm.c`** — a minimal data-directive-only assembler (defines
  `CONFIG_TCC_ASM`; the generic `.int`/`.word`/label directives come from
  `tccasm.c`, and the instruction-level hooks are stubs that error). Wired in via
  the `arm64-asm-{defs,include}` simple-patches. This lets the primitives be
  written as raw aarch64 instruction words: `__asm__(".int 0x..")`, exactly like
  mes' existing 32-bit arm `__TINYC__` path.

### libc pieces added (in the `mes/` submodule)

- `lib/linux/aarch64-mes-gcc/{crt1,crti,crtn,_exit,syscall,_write}.c` — runtime
  primitives as raw words. The Linux/aarch64 syscall ABI is number in `x8`, args
  `x0..x5`, `svc #0`, result `x0`; `crt1.c` reads argc/argv/envp off the entry
  stack (at `x29+224`, given tcc's `stp x29,x30,[sp,#-224]!` prologue).
- `lib/aarch64-mes-gcc/setjmp.c` — `setjmp`/`longjmp` (best-effort, like the arm
  `__TINYC__` path; only needs to *compile* for Milestone 2).
- `include/linux/aarch64/{syscall,kernel-stat,signal}.h` — asm-generic syscall
  numbers and `struct stat` (identical to riscv64). The `*at`-only file syscalls
  are reached through mes' existing `#elif defined(SYS_*at)` fallbacks.
- `include/stdint.h` / `include/setjmp.h` — LP64 `LONG_MAX`/`SIZE_MAX` and the
  `__aarch64__` `__jmp_buf` layout.

### stdarg / va_list (the subtle part)

`arm64-gen.c` already implements the AAPCS64 va mechanism via the native
`__va_start`/`__va_arg` builtins, so no `va_list.c` is needed (unlike the x86_64
port). But tcc's native `__va_arg` takes the **address** of its operand
(`gen_va_arg` does `gaddrof`), which only works when the operand is the va_list
storage itself. mes forwards a va_list across functions (`printf` → `vprintf` →
`vfprintf`); since `va_list` is an array (`va_list[1]`, per AAPCS64), the callee
receives a *pointer*, and `__va_arg` then reads the address of that pointer —
fetching garbage. So `include/stdarg.h` keeps the native `__va_start` (only ever
applied to a true local array) but routes `va_arg` through small C helpers
(`lib/aarch64-mes-gcc/va.c`) that take the decayed `__va_list *`, which is
uniform whether the va_list is local or forwarded.

### A latent seed-compiler bug

`tcc_cc` (the limited seed compiler) miscompiles tcc's `parse_builtin_params`
loop — `while ((c = *args++)) { switch (c) { …; continue; } }` — on its *second*
iteration, mis-dispatching any 2-argument builtin (e.g. `__va_start`) to the
`internal error` default. The x86_64 bootstrap never hit this (it uses a `char*`
va_list, not the arm64 builtins). The `arm64-va-builtin-loop` simple-patch
rewrites the loop as an indexed `for` + `if`/`else`, which `tcc_cc` compiles
correctly.

### Soft-float

The arm64 backend emits libcalls (`__extenddftf2`, `__addtf3`, …) for
`long double` (binary128). tcc ships these in `lib/lib-arm64.c`; `build_arm64.sh`
compiles it into `libtcc1.a` (the arm64 analogue of libgcc's TFmode helpers).

## Milestone 3 — boot0/1/2 + fixpoint

`build_arm64.sh` now mirrors the amd64 boot loop. The seed `tcc_s` compiles
`tcc.c` into `tcc-boot0`; each stage then rebuilds the full libc set
(`crt1.o`, `libc.a`, `libtcc1.a`, `libgetopt.a`) with the freshly built compiler
and compiles the next stage. Every compile/link runs under `qemu-aarch64-static`
via a one-line `_run` wrapper (`make_run_wrapper`) so each boot binary acts as a
single-token compiler. Success is the same criterion as amd64: `cmp tcc-boot2
tcc-boot3` byte-identical.

```
$ ./build_arm64.sh
...
tcc version 0.9.26 (AArch64 Linux)      # tcc-boot0 -version
tcc version 0.9.26 (AArch64 Linux)      # tcc-boot1 -version
tcc version 0.9.26 (AArch64 Linux)      # tcc-boot2 -version
Hello, World!                           # hello rebuilt by tcc-boot2
Variadic: answer = 42
FIXPOINT: tcc-boot2 == tcc-boot3 (identical)
```

`tcc-boot1`, `tcc-boot2`, `tcc-boot3` are byte-identical (1109132 bytes);
`tcc-boot0` differs only because it is the lone stage built by the seed pipeline.

### aarch64 `setjmp`/`longjmp` and the caller's frame

`tcc_compile` wraps its body in `if (setjmp(s1->error_jmp_buf) == 0)`, so the
boot loop is the first runtime exercise of the mes `setjmp`/`longjmp`. Since tcc
0.9.26 has no arm64 assembler these are raw aarch64 instruction words, and they
have to cooperate with the prologue/epilogue tcc generates around them.

The subtlety is that a hand-written `setjmp` must **not** return through a bare
`ret`. tcc's prologue pushes the caller's `x29`/`x30` and repoints the frame
pointer `x29` at `setjmp`'s own frame; a bare `ret` would leave `sp`/`x29`
pointing there, and the caller would then read its locals (e.g. `tcc_compile`'s
`TCCState *`) off the wrong frame. So `setjmp` saves the *caller's* frame — fp
from `[x29]`, lr from `x30`, sp from `x29 + frame_size` — and returns `0` via a
normal C `return`, letting tcc emit its real epilogue to restore the caller;
`longjmp` reloads those saved slots and `ret`s straight into setjmp's caller.
(The one tcc-chosen constant, the frame size, is verified by re-disassembling the
compiled `setjmp`.)
