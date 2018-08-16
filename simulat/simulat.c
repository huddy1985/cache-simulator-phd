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

int valid_cache_size(int size)
{
    if (!log2(size))
        return -1;


}

int log2(int num)
{
    return !(num & (num - 1));
}
