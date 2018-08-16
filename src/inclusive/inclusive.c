/*
 * @file config.h
 * @author charlies
 * @mail: xuguo.wong@gmail.com
 * @date 2018/08/13
 *
 * authority: GPL v2.0
 *
 * This is the simulator of inclusive cache.
 */

#include "cache_ops.h"
#include "cache.h"

int inclusive_init ( cache_level_t, size_t);
cache_H_M_category_t inclusive_load (long, const char *, size_t);
cache_H_M_category_t inclusive_writeback (long , const char *, size_t);
void inclusive_invalid (cache_level_t, long);
void inclusive (cache_level_t, long, const char *, size_t);

cache_operations_t cache_inclusive = {
    .init = inclusive_init,
    .load = inclusive_load,
    .writeback = inclusive_writeback,
    .invalid = inclusive_invalid,
    .inclusive = inclusive,
};

int inclusive_init ( cache_level_t level, size_t size)
{
    return 0;
}

cache_H_M_category_t
inclusive_load (long address, const char *data, size_t size)
{
    return CHMC_hit;
}

cache_H_M_category_t
inclusive_writeback (long address, const char *data, size_t size)
{
    return CHMC_hit;
}

void inclusive_invalid (cache_level_t level, long address)
{
    
}

void
inclusive (cache_level_t level, long address, const char *data, size_t size)
{
    
}

