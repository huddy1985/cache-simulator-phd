/*
 * for testing the libud.so
 */
#include "udis86.h"
#include <string.h>

int input_hook_x(ud_t* u);

int main()
{
    ud_t u;

    ud_init(&u);
    ud_set_input_file(&u, stdin);
    ud_set_mode(&u, 64);
    ud_set_syntax(&u, UD_SYN_INTEL);

    ud_set_input_hook(&u, input_hook_x);

    while (ud_disassemble(&u)) {
        printf("%016lx ", ud_insn_off(&u));
        const char *hex1, *hex2;
        hex1 = ud_insn_hex(&u);
        hex2 = hex1 + 16;
        printf("%-16.16s %-24s", hex1, ud_insn_asm(&u));
        if (strlen(hex1) > 16) {
            printf("\n");
            printf("%15s", "");
            printf("%-16s", hex2);
        }

    }
}

int input_hook_x(ud_t* u)
{
    unsigned int c, i;

    i = fscanf(stdin, "%x", &c);

    if (i == EOF)
        return UD_EOI;
    if (i == 0) {
        fprintf(stderr, "Error: Invalid input, should be in hexadecimal form (8-bit).\n");
        return UD_EOI;
    }
    if (c > 0xFF)
        fprintf(stderr, "Warning: Casting non-8-bit input (%x), to %x.\n", c, c & 0xFF);
    return (int) (c & 0xFF);
}
