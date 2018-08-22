// cfg.c
// A Control Flow Graph basic block generator for use with the
// libdisasm x86 disassembler, written in C.
// 
// Joe Mullally 2011, released into the Public Domain.
// 
// 
// To produce a graph of the control flow basic blocks:
//  - Disassemble the x86 program text into an array of disassembled 
//        instructions with x86_disasm(...).
//  - Pass this array to cfg_make(...).
//  - Use the returned lists to navigate through the graph.
// 
//  See test_cfg.c for sample usage, and test_cfg.out, graph.dot and graph.png
//  for what the constructed graph looks like.
// 
// 
//  cfg_make() returns a linked list of all basic blocks, each of which contains
//  the instruction range the block covers, along with a list of connected
//  parent and child blocks. A child block is one that program control flow
//  progresses into from a parent block.
//  A list of the top level nodes (ie nodes with no parents) is also returned.
// 
//  x86_disasm() and cfg_make() should be able to decode most of the structure
//  in typical x86 ELF relocatable executables.
// 
//  For now the types of branching supported are static relative jumps,
//  Tracking possible indirect jumps through register values is a much
//  harder problem that requires data flow analysis.
//  This code also depends on being supplied a linear scan disassembly
//  of the x86 instructions. In practice an x86 program can jump to and start
//  executing at any byte. If there are extra bytes between the instructions
//  that wouldn't normally be executed this can confuse the linear disassembly
//  approach here.
//  This shouldn't be a problem with typical compiler generated
//  ELF .text sections.
// 
// 
// Porting:
//   It should be easy to convert this to a dissassembler library other
//   than libdisasm.
// 
//   To make the CFG, each disassembled instruction needs:
//     - size of instruction in bytes
//     - a way of checking if the instruction is a branch instruction
//     - if it is a branch instruction, a way of getting the branch target
//       offset from the instruction.
// 
//  Most of the library specific changes will likely be in:
//     is_branch_inst(), is_deadend_inst(), get_branch_targets()
// 
// Future:
//   Add absolute address jumps.
//   Maybe add a call and callee list field to the basic block structure to
//    track call instructions.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <libdis.h>

#include "cfg.h"


static void list_append(void *list, void *item);
static void list_free(void *list);

struct st_int_list {
    struct st_int_list *next;
    int val;
};

struct st_branch {
    struct st_branch *next;
    int rel_offset;
};


static int is_branch_inst(x86_insn_t *inst)
{
    // We only care about jumps, as calls normally return execution to the 
    // position they are called from and hence don't change control flow.
    if (inst->type == insn_jmp || inst->type == insn_jcc)
        return 1;
    return 0;
}

static int is_deadend_inst(x86_insn_t *inst)
{
    if (inst->type == insn_return)
        return 1;
    return 0;
}

static struct st_branch * get_branch_targets(x86_insn_t *inst)
{
    struct st_branch *branches;
    x86_op_t *op;
    char disbuf[256];
    disbuf[255] = '\0';

    assert (inst->type == insn_jmp || inst->type == insn_jcc);

    op = x86_get_branch_target(inst);

    if (op->type != op_relative_near && op->type != op_relative_far) {
        // Leave absolute, indirection, address expressions unsupported for now.
        printf("Warning: Branching address type not handled, ignoring...\n");
        x86_format_insn (inst, disbuf, 255, att_syntax);
        printf("           %08x [%2i] %s\\l", inst->addr, inst->size, disbuf);
        return NULL;
    }

    branches = malloc(sizeof(struct st_branch));
    branches->next = NULL;

    if (op->type == op_relative_near) {
        branches->rel_offset = op->data.relative_near;
    } else if (op->type == op_relative_far) {
        branches->rel_offset = op->data.relative_far;
    }
    if (inst->type == insn_jcc) {
        branches->next = malloc(sizeof(struct st_branch));
        branches->next->rel_offset = 0; // Point to next instruction
        branches->next->next = NULL;
    }
    return branches;
}


// Make a basic control flow graph for the inputted array of decoded 
// instructions.
// Return a list of all basic blocks in the CFG, as well as a list
// containing the top-level nodes (ie nodes with no other parent blocks).
void cfg_make(x86_insn_t **insts, int insts_len, 
				struct cfg_node_list **nodelist_ret, 
				struct cfg_node_list **top_nodes_ret)
{
    int inst_idx, inst_target_idx, end_block, byte_idx, inst_byte_idx, 
        target_offset, text_len, *map_offset_to_idx;
    struct st_int_list **parents, **children, *parenti, *childi;
    struct st_branch *branches, *branch;
    struct cfg_node_list *top_nodes, *topnode_ll, *nodelist, *node_ll, 
                         *parentb, *childb;
    struct cfg_node *cfgnode, **idx_to_block;


    parents = malloc(insts_len * sizeof(struct st_int_list));
    children = malloc(insts_len * sizeof(struct st_int_list));
    idx_to_block = malloc(insts_len * sizeof(struct cfg_node *));

    for (inst_idx = 0; inst_idx < insts_len; inst_idx++) {
        parents[inst_idx] = NULL;
        children[inst_idx] = NULL;
    }

    // Build an easy instruction offset -> idx map
    text_len = 0;
    for (inst_idx = 0; inst_idx < insts_len; inst_idx++)
        text_len += insts[inst_idx]->size;
    map_offset_to_idx = malloc(text_len * sizeof(int));
    inst_idx = 0;
    inst_byte_idx = 0;
    for (byte_idx = 0; byte_idx < text_len; byte_idx++) {
        map_offset_to_idx[byte_idx] = inst_idx;
        inst_byte_idx++;
        if (inst_byte_idx == insts[inst_idx]->size) {
            inst_byte_idx = 0;
            inst_idx++;
        }
    }

    // Mark start and end of basic blocks
    // Set parent marker when another block jumps here.
    // Set children when this instruction branches to 
    byte_idx = 0;
    for (inst_idx = 0; inst_idx < insts_len; inst_idx++) {
        if (is_branch_inst(insts[inst_idx])) {

            branches = get_branch_targets(insts[inst_idx]);
            for (branch = branches; branch != NULL; branch = branch->next) {
                target_offset = byte_idx + insts[inst_idx]->size + branch->rel_offset;
                if (target_offset < 0 || target_offset >= text_len) {
                    printf("Warning: Instruction [%i] control flow jump %i outside of"
                           " text region, ignoring...\n", inst_idx, target_offset);
                    continue;
                }
                inst_target_idx = map_offset_to_idx[target_offset];
                
                // Make this instruction/block the parent of the target block
                // The target instruction is the beginning of a basic block.
                parenti = malloc(sizeof(struct st_int_list));
                parenti->val = inst_idx;
                list_append(&parents[inst_target_idx], parenti);

                // Make the target block the child of this instruction/block
                // This branch instruction is the end of a basic block.
                childi = malloc(sizeof(struct st_int_list));
                childi->val = inst_target_idx;
                list_append(&children[inst_idx], childi);
            }
            list_free(branches);
        }
        byte_idx += insts[inst_idx]->size;
    }

    // Construct the basic blocks, using the regions between the
    // parents and children markers as the start and end of each block
    nodelist = NULL;
    for (inst_idx = 0; inst_idx < insts_len; inst_idx++) {

        node_ll = malloc(sizeof(struct cfg_node_list));
        list_append(&nodelist, node_ll);

        cfgnode = malloc(sizeof(struct cfg_node));
        node_ll->node = cfgnode;
        cfgnode->parents = NULL;
        cfgnode->children = NULL;
        cfgnode->start_inst = inst_idx;
        cfgnode->block_len = 1;
        idx_to_block[inst_idx] = cfgnode;

        if (children[inst_idx] != NULL || is_deadend_inst(insts[inst_idx]) == 1)     
            continue;        // This single instruction is in a block of its own
        end_block = 0;
        inst_idx++;
        while (inst_idx < insts_len && end_block == 0) {
            if (parents[inst_idx] != NULL) {
                // We hit the next block without seeing this blocks children 
                // first. It looks like another block jumps into the middle of
                // this one. Split this block and make this block the parent 
                // of the next block which will contain the next instruction.
                parenti = malloc(sizeof(struct st_int_list));
                parenti->val = inst_idx - 1;
                list_append(&parents[inst_idx], parenti);

                childi = malloc(sizeof(struct st_int_list));
                childi->val = inst_idx;
                list_append(&children[inst_idx-1], childi);

                inst_idx--;
                end_block = 1;
            } 
            else if (children[inst_idx] != NULL
                    || is_deadend_inst(insts[inst_idx]) == 1) {
                // This instruction is a branch out from this block,
                // or a return at the end of the block.
                // Include the instruction and continue to the next block.
                idx_to_block[inst_idx] = cfgnode;
                cfgnode->block_len++;
                end_block = 1;
            } 
            else {
                // We are neither branching out or jumping into the code at this
                // point, so include in this block.
                idx_to_block[inst_idx] = cfgnode;
                cfgnode->block_len++;
                inst_idx++;
            }
        }
    }


    // Iterate through all the basic blocks, and fill in the pointers to
    // the parent and children blocks.
    top_nodes = NULL;
    node_ll = nodelist;
    while (node_ll != NULL) {
        cfgnode = node_ll->node;

        // If this block has no parents, it is a top level node.
        if (parents[cfgnode->start_inst] == NULL) {
            topnode_ll = malloc(sizeof(struct cfg_node_list));
            topnode_ll->node = cfgnode;
            list_append(&top_nodes, topnode_ll);
        } else {
            parenti = parents[cfgnode->start_inst];
            while (parenti != NULL) {
                parentb = malloc(sizeof(struct cfg_node_list));
                parentb->node = idx_to_block[parenti->val];
                list_append(&cfgnode->parents, parentb);
                parenti = parenti->next;
            }
        }

        childi = children[cfgnode->start_inst + cfgnode->block_len - 1];
        while (childi != NULL) {
            childb = malloc(sizeof(struct cfg_node_list));
            childb->node = idx_to_block[childi->val];
            list_append(&cfgnode->children, childb);
            childi = childi->next;
        }

        node_ll = node_ll->next;
    }

    // Free working memory
    for (inst_idx = 0; inst_idx < insts_len; inst_idx++) {
        list_free(parents[inst_idx]);
        list_free(children[inst_idx]);
    }
    free(parents);
    free(children);
    free(idx_to_block);

    *nodelist_ret = nodelist;
    *top_nodes_ret = top_nodes;

    return;
}

// Free a CFG node list returned from cfg_make()
void cfg_free_list(struct cfg_node_list *node_list)
{
    list_free(node_list);
    return;
}

// Free a CFG node list and blocks returned from cfg_make(). 
void cfg_free_list_and_blocks(struct cfg_node_list *node_list)
{
    struct cfg_node_list *node_ll, *node_ll_next;
    struct cfg_node *cfgnode;
    node_ll = node_list;
    while (node_ll != NULL) {
        cfgnode = node_ll->node;
        list_free(cfgnode->parents);
        list_free(cfgnode->children);
        free(cfgnode);
        node_ll_next = node_ll->next;
        free(node_ll);
        node_ll = node_ll_next;
    }
    return;
}


// Simple CFG output suitable for debugging.
void cfg_print(struct cfg_node_list *node_list)
{
    struct cfg_node_list *nl, *nll;
    struct cfg_node *cfgnode;
    nl = node_list;
    while (nl != NULL) {
        cfgnode = nl->node;
        printf("node: %p, %i -> %i\n  parents:", cfgnode, cfgnode->start_inst, 
                cfgnode->start_inst + cfgnode->block_len - 1);
        for (nll = cfgnode->parents; nll != NULL; nll = nll->next)
            printf(" %p,", nll->node);
        printf("\n  children:");
        for (nll = cfgnode->children; nll != NULL; nll = nll->next)
            printf(" %p,", nll->node);
        printf("\n");
        nl = nl->next;
    }
    printf("\n");
    return;
}


// Output the complete CFG node list as a graphviz graph.
// Render with: dot -Tps output.dot -o output.ps
void cfg_fprint_graphviz(FILE *outfile, struct cfg_node_list *node_list)
{
    struct cfg_node_list *nl, *nll;
    struct cfg_node *cfgnode;
    nl = node_list;
    fprintf(outfile, "digraph G\n{\n");
    while (nl != NULL) {
        cfgnode = nl->node;
        for (nll = cfgnode->children; nll != NULL; nll = nll->next) {
            fprintf(outfile, "    n%i_%i -> n%i_%i;\n", 
                    cfgnode->start_inst, 
                    cfgnode->start_inst + cfgnode->block_len - 1, 
                    nll->node->start_inst, 
                    nll->node->start_inst + nll->node->block_len - 1);
        }
        nl = nl->next;
    }
    fprintf(outfile, "}\n");
    return;
}


// Output the complete CFG node list as a graphviz graph.
// Render with: dot -Tps output.dot -o output.ps
// Label the graph nodes with the disassembled instructions.
void cfg_fprint_graphviz_insts(FILE *outfile, struct cfg_node_list *node_list,
								 x86_insn_t **insts, int insts_len)
{
    struct cfg_node_list *nl, *nll;
    struct cfg_node *cfgnode;
    char disbuf[1024], *tabchr;
    int inst_idx;
    disbuf[1023] = '\0';

    nl = node_list;
    fprintf(outfile, "digraph G\n{\n");
    while (nl != NULL) {
        cfgnode = nl->node;
        fprintf(outfile, "    n%i_%i [ shape = \"box\"\n"
                         "             fontname = \"Monospace\"\n"
                         "             label = \"",
						cfgnode->start_inst, 
						cfgnode->start_inst + cfgnode->block_len - 1);
        for(inst_idx = cfgnode->start_inst; 
			inst_idx < cfgnode->start_inst + cfgnode->block_len; 
			inst_idx++) {
            x86_format_insn (insts[inst_idx], disbuf, 1023, att_syntax);
            while ((tabchr = strchr(disbuf, '\t')) != NULL)
                *tabchr = ' ';      // remove tab chars, graphviz doesn't like them
            fprintf(outfile, "%6i : %08x [%2i] %s\\l", 
										inst_idx, 
										insts[inst_idx]->addr, 
										insts[inst_idx]->size, disbuf);
        }
        fprintf(outfile, "\" ];\n");
        for (nll = cfgnode->children; nll != NULL; nll = nll->next) {
            fprintf(outfile, "      n%i_%i -> n%i_%i;\n", 
                    cfgnode->start_inst, 
                    cfgnode->start_inst + cfgnode->block_len - 1, 
                    nll->node->start_inst, 
                    nll->node->start_inst + nll->node->block_len - 1);
        }
        nl = nl->next;
    }
    fprintf(outfile, "}\n");
    return;
}


//////// Utility functions ////////
// Some list helper functions.

// Append an item to the end of a linked list.
// Needs the first entry in the list item struct to be the 'next' pointer.
//
// Pass a pointer to the variable holding the first node (head) of the list.
// e.g. list_append(&head, item);
// (Replacing this with a tail caching version:
//       list_append(&list, item, (void *) &tailcache) 
//  only gives a tiny speedup for our small uses here).
static void list_append(void *list, void *item)
{
    void **next = (void **) list;
    assert (list != NULL);
    while (*next != NULL)
        next = *next;
    *next = item;
    *(void **)item = NULL;
    return;
}

// Free a list: Call free() on each item in a linked list.
// Needs the first entry in list item struct to be the 'next' pointer.
static void list_free(void *list)
{
    void *next, *cur = list;
    while (cur != NULL) {
        next = *(void **) cur;
        free(cur);
        cur = next;
    }
    return;
}

//////// END Utility functions ////////


