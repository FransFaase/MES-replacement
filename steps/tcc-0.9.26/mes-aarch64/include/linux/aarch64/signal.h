/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 *
 * This file is part of GNU Mes.
 *
 * GNU Mes is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU Mes is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Mes.  If not, see <http://www.gnu.org/licenses/>.
 */

// Taken from musl libc (aarch64)

typedef struct sigcontext
{
  unsigned long fault_address;
  unsigned long regs[31];
  unsigned long sp;
  unsigned long pc;
  unsigned long pstate;
  long double __reserved[256];
} mcontext_t;

struct sigaltstack
{
  void *ss_sp;
  int ss_flags;
  size_t ss_size;
};

typedef struct __ucontext
{
  unsigned long uc_flags;
  struct __ucontext *uc_link;
  stack_t uc_stack;
  sigset_t uc_sigmask;
  mcontext_t uc_mcontext;
} ucontext_t;
