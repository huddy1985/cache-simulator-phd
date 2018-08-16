/*
 * @file config.h
 * @author charlies
 * @mail: xuguo.wong@gmail.com
 * @date 2018/08/13
 *
 * authority: GPL v2.0
 *
 * The real simulat action function.
 */

#ifndef __ELF_PARSER_H__
#define __ELF_PARSER_H__

typedef register {
#ifdef __x86_64__
    union {
        long long rax;
        long eax;
        short ax;
        char ah,al;
    };
    union {
        long long rbx;
        long ebx;
        short bx;
        char bh,bl;
    };
    union {
        long long rcx;
        long ecx;
        short cx;
        char ch,cl;
    };
    union {
        long long rdx;
        long edx;
        short dx;
        char dh,dl;
    };
    
#else

#endif
}

typedef elf_stream {
    long address;
    char data[];
} elf_stream_t;

typedef elf_zone {
    long address;
    elf_stream_t stream[];
} elf_zone_t;

typedef elf_section {
    long address;
    elf_zone_t zones[];
} elf_secton_t;

typedef elf_obj {
    int arch: 4;
    int 
}

#endif /* __ELF_PARSER_H__ */
