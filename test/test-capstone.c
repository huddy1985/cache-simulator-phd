#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <platform.h>
#include <capstone.h>

#include <bfd.h>

struct platform {
	cs_arch arch;
	cs_mode mode;
	unsigned char *code;
	size_t size;
	char *comment;
	cs_opt_type opt_type;
	cs_opt_value opt_value;
};

void error_exit(int status, int errnum, const char *format, ...)
{
    va_list ap; 
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    if (errnum != 0) {
        errno = errnum;
        perror("");
    }   
    exit(status);
    return;
}

static void print_string_hex(char *comment, unsigned char *str, size_t len)
{
	unsigned char *c;
	printf("%s", comment);
	for (c = str; c < str + len; c++) {
		printf("0x%02x ", *c & 0xff);
	}
	printf("\n");
}

static void print_insn_detail(csh ud, cs_mode mode, cs_insn *ins, csh handle)
{
    int count, i;
    cs_x86 *x86;

    // detail can be NULL on "data" instruction if SKIPDATA option is turned ON
    if (ins->detail == NULL)
        return;

    x86 = &(ins->detail->x86);
	
	if (strlen(x86->prefix))
    	print_string_hex("\tPrefix:", x86->prefix, strlen(x86->prefix));
	
	if (strlen(x86->opcode))
    	print_string_hex("\tOpcode:", x86->opcode, strlen(x86->opcode));

    printf("\trex: 0x%x\n", x86->rex);

    printf("\taddr_size: %u\n", x86->addr_size);
    printf("\tmodrm: 0x%x\n", x86->modrm);
    printf("\tdisp: 0x%x\n", x86->disp);

    // SIB is not available in 16-bit mode
    if ((mode & CS_MODE_16) == 0) {
        printf("\tsib: 0x%x\n", x86->sib);
        if (x86->sib_base != X86_REG_INVALID)
            printf("\t\tsib_base: %s\n", cs_reg_name(handle, x86->sib_base));
        if (x86->sib_index != X86_REG_INVALID)
            printf("\t\tsib_index: %s\n", cs_reg_name(handle, x86->sib_index));
        if (x86->sib_scale != 0)
            printf("\t\tsib_scale: %d\n", x86->sib_scale);
    }

    for (i = 0; i < x86->op_count; i++) {
        cs_x86_op *op = &(x86->operands[i]);

        switch((int)op->type) {
            case X86_OP_REG:
                printf("\t\toperands[%u].type: REG = %s\n", i, cs_reg_name(handle, op->reg));
                break;
            case X86_OP_IMM:
                printf("\t\toperands[%u].type: IMM = 0x%" PRIx64 "\n", i, op->imm);
                break;
            case X86_OP_MEM:
                printf("\t\toperands[%u].type: MEM\n", i);
                if (op->mem.segment != X86_REG_INVALID)
                    printf("\t\t\toperands[%u].mem.segment: REG = %s\n", i, cs_reg_name(handle, op->mem.segment));
                if (op->mem.base != X86_REG_INVALID)
                    printf("\t\t\toperands[%u].mem.base: REG = %s\n", i, cs_reg_name(handle, op->mem.base));
                if (op->mem.index != X86_REG_INVALID)
                    printf("\t\t\toperands[%u].mem.index: REG = %s\n", i, cs_reg_name(handle, op->mem.index));
                if (op->mem.scale != 1)
                    printf("\t\t\toperands[%u].mem.scale: %u\n", i, op->mem.scale);
                if (op->mem.disp != 0)
                    printf("\t\t\toperands[%u].mem.disp: 0x%" PRIx64 "\n", i, op->mem.disp);
                break;
            default:
                break;
        }


        // AVX broadcast type
        if (op->avx_bcast != X86_AVX_BCAST_INVALID)
            printf("\t\toperands[%u].avx_bcast: %u\n", i, op->avx_bcast);

        // AVX zero opmask {z}
        if (op->avx_zero_opmask != false)
            printf("\t\toperands[%u].avx_zero_opmask: TRUE\n", i);

        printf("\t\toperands[%u].size: %u\n", i, op->size);
    }

    printf("\n");
}



int main(int argc, const char **argv)
{
	bfd *elf_file;
	asection *text_section;
	int text_len = 0, err = 0;
	unsigned char *text_content;
	uint64_t textaddr;

    printf("<usage>: test binaryfile");

	

	uint64_t address;
	cs_insn *insn;
	int i;
	size_t count;
	cs_err cs_err;
	csh handle;

    elf_file = bfd_openr(argv[1], NULL);
    if (!elf_file) {
        error_exit(-1, errno, "Unable to open binary file, [%s]", argv[1]);
    }

    bfd_check_format(elf_file, bfd_object);
	text_section = bfd_get_section_by_name(elf_file, ".text");
	if (!text_section) {
		error_exit(-1, errno, "Unable to read .text from file, [%s]", argv[1]);
	}

	text_len = bfd_get_section_size(text_section);
	text_content = malloc(text_len);
	err = bfd_get_section_contents(elf_file, text_section, text_content, 0, text_len);
	if (err == 0) {
		error_exit(-1, errno, "Unable to get text content from file, [%s]", argv[1]);
	}

	textaddr = bfd_get_section_vma(elf_file, text_section);

	struct platform platforms[] = {
		{
			CS_ARCH_X86,
			CS_MODE_64,
			(unsigned char*) text_content,
			text_len,
			"X86_64 (Intel syntax)",
		},
	};
	
	address = textaddr;
	cs_err = cs_open(platforms[0].arch, platforms[0].mode, &handle);	
	if (cs_err)
		error_exit(-2, errno, "Cannot init platform of cs, [%s]", argv[1]);

	if (platforms[0].opt_type)
		cs_option(handle, platforms[0].opt_type, platforms[0].opt_value);

	cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

	count = cs_disasm(handle, platforms[0].code, platforms[0].size, address, 0, &insn);
	if (count) {
		size_t i;
		puts("**************");
		printf("platform %s\n", platforms[0].comment);
		print_string_hex("Code:", platforms[0].code, platforms[0].size);
		printf("Disasm:\n");
		
		for (i = 0; i < count; i++) {
			printf("0x%" PRIx64 ":\t%s\t%s\n", insn[i].address, insn[i].mnemonic, insn[i].op_str);
			print_insn_detail(handle, platforms[0].mode, &insn[i], handle);
		}
		
		printf("0x%" PRIx64 ":\n", insn[i - 1].address + insn[i - 1].size);
		cs_free(insn, count);
	} else {
		printf("************");
		printf("Platform %s\n", platforms[0].comment);
		print_string_hex("Code:", platforms[0].code, platforms[0].size);
		printf("ERROR: Failed to disasm given code\n");
	}

	printf("\n");
	cs_close(&handle);
}









