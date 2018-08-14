/*
 * @file config.h
 * @author charlies
 * @mail: xuguo.wong@gmail.com
 * @date 2018/08/13
 *
 *
 * Here is a cache simulator configuration header.
 */
#ifndef __CACHE_OPS_H__
#define __CACHE_OPS_H__
#include "cache.h"

typedef struct cache_operations {
    int (*init) (cache_level_t, size_t size);
    cache_H_M_category_t (*load) (long address, const char *data, size_t size);
    cache_H_M_category_t (*writeback) (long address, const char *data, size_t size);
    void (*invalid) (cache_level_t level, long address);
    void (*inclusive) (cache_level_t level, long address, const char *data, size_t size);
    void (*exclusive) (cache_level_t level, long address, const char *data, size_t size);
    void (*NINE)  (cache_level_t level, long address, const char *data, size_t size);
} cache_operations_t;

#endif /* __CACHE_OPS_H__ */