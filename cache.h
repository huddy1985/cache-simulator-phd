/*
 * @file list.h
 * @author charlies
 * @date 2018/08/13
 *
 *
 * Here is a recipe to cook list.h for user space program.
 * 1. copy list.h from linux/include/list.h
 * 2. remove
 *     - #ifdef __KERNE__ and its #endif
 *     - all #include line
 *     - prefetch() and rcu related functions
 * 3. add macro offsetof() and container_of
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <list.h>
#include <stdio.h>
#include <stdlib.h>

// The current cpu architecture has 2 type of cache type:
// 		1 ICache, store the instructions
// 		2 DCache, store the data
typedef enum cache_type {
    ICache = 0,
    DCache,
} cache_type_t;

// The current cpu architecture most has multi-level cache,
// E.g. the cpu i-serial has 3 level cache,
// L1, L2, L3, L1 is the top level, the L3 is the down-est
// level, L1 are privacy of a core of cpu, The L2 always
// share in a socket cpu. L3 is the cache all of cpus shared.
typedef enum cache_level {
    L1 = 1,
    L2,
    L3,
    L4,
} cache_level_t;

typedef enum cache_lv_size {
    unknown = 0, 	// not a standard capacity
    S_16KB = 1,
    S_32KB = 2,
    S_64KB = 3,
    S_128KB = 4,
    S_1MB = 5,
    S_2MB = 6,
    S_4MB = 7,
    S_8MB = 8,
    S_16MB = 9,
} cache_lv_size_t;

// please read this site:
// https://blog.csdn.net/dongyanxia1000/article/details/53392315
typedef enum cache_set_associative {
    unknown = 0,
    SA_direct_map,
    SA_full_map,
    SA_set_2,
    SA_set_4,
    SA_set_8,
    SA_set_16
} cache_set_associative_t;

typedef enum cache_set_ways {
    unknown = 0,
    SW_2ways = 1,
    SW_4ways = 2,
    SW_8ways = 3,
    SW_16ways = 4,
    SW_32ways = 5,
    SW_64ways = 6,
} cache_set_ways_t;

// Please read this site:
// https://en.wikipedia.org/wiki/Cache_inclusion_policy
typedef enum cache_hierachy_policy {
    unknown = 0,
    H_inclusive = 1,
    H_exclusive = 2,
    H_non_exclusive = 3,
} cache_hierachy_policy_t;

// please read this site:
// https://en.wikipedia.org/wiki/Cache_replacement_policies
typedef enum cache_conservative_policy {
    unknown = 0,
    CP_random,
    CP_lru,
    CP_fifo,
    CP_lifo,
    CP_tlru,
    CP_mru,
} cache_convervative_policy_t;

typedef struct cache {
    struct list_head list;
    cache_type_t t_cache;
    cache_level l_cache;
    cache_hierachy_policy_t hp_cache;
    cache_conservative_policy cp_cache;
    cache_set_associative_t	sa_cache;
    cache_set_ways_t sw_cache;
    char data[];
} cache_t;

typedef struct cache_operations {
    void invalid(int level, cache_t *cache);
    void init(int level, cache_t *cache);
    void replace(int level, cache_t *cache);
} cache_ops_t;

#endif /* __CONFIG_H__ */
