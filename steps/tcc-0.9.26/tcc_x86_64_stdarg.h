/* tcc_x86_64_stdarg.h - x86_64 TCC stdarg for the MES bootstrap.
   Replaces the i386 "char* va_list" convention with the System V AMD64
   __va_list_struct / reg_save_area mechanism. __va_start and __va_arg are
   provided by va_list.o (tcc lib/va_list.c) which is linked into libc.a. */
#ifndef __TCC_X86_64_STDARG_H
#define __TCC_X86_64_STDARG_H 1

typedef struct {
    unsigned int gp_offset;
    unsigned int fp_offset;
    union {
        unsigned int overflow_offset;
        char *overflow_arg_area;
    };
    char *reg_save_area;
} __va_list_struct;

typedef __va_list_struct va_list[1];

void __va_start(__va_list_struct *ap, void *fp);
void *__va_arg(__va_list_struct *ap, int arg_type, int size, int align);

/* arg_type: 0 = GP register, 1 = XMM register, 2 = stack */
#define va_start(ap, last) __va_start(ap, __builtin_frame_address(0))
#define va_arg(ap, type)   (*(type *)(__va_arg(ap, 0, sizeof(type), __alignof__(type))))
#define va_arg8(ap, type)  (*(type *)(__va_arg(ap, 1, sizeof(type), __alignof__(type))))
#define va_copy(dest, src) (*(dest) = *(src))
#define va_end(ap)

typedef va_list __gnuc_va_list;
#define _VA_LIST_DEFINED
#define __DEFINED_va_list

#endif /* __TCC_X86_64_STDARG_H */
