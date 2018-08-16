/*
 * @file config.h
 * @author charlies
 * @mail: xuguo.wong@gmail.com
 * @date 2018/08/13
 *
 * authority: GPL v2.0
 *
 * The simulat function definition header.
 */

/*
 * Basic function.
 * verify whether a size number is pow of 2,
 * for verify cache size.
 */
#ifndef __SIMULAT_H__
#define __SIMULAT_H__
#include "cache.h"
#include "elf_parser.h"

int log2(int num);

/*
 * wrapper of log2 function.
 */
int valid_cache_size(int size);

/*
 * prepare simulate function.
 * concrete the global obj of cache.
 * cache_t cache    [in,out] 	: the global of cache obj
 * int arch             [in]	: type of arch, i386, x86_64
 * int type             [in]	: type of cache hardware type, e.g. ICache
 * int level            [in]  : how many level of this cache arch
 * int linesize         [in]  : how many of cache block size
 * const char * lvsize  [in]  : e.g. [32k, 64k]
 * const char *sw       [in]  : ways of cache
 * const char *hierarchy[in]  : inclusive or exclusive or NINE
 * const char *policy   [in]  : lru, fifo ...
 */
int prepare_simulate(cache_t **cache, int arch, int type, int level, int linesize,
             const char *lvsize, const char *sw, const char *hierarchy,
             const char *policy);

int prepare_elf_data(FILE *elffile, elf_section_t *elf);
/*
 * 
 */
int run(cache_t *cache, );

#endif /* __SIMULAT_H__ */
