/*
 * @file config.h
 * @author charlies
 * @mail: xuguo.wong@gmail.com
 * @date 2018/08/13
 *
 * authority: GPL v2.0
 *
 * Here is a cache simulator configuration header.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cfg.h"
#include "cache.h"
#include "list.h"

static char *cfg_file = "conf/cfg.cache";

cache_t g_cache;

/* for read the data from config file */
int cfg_type;
int cfg_level;
int cfg_linesize;
int cfg_arch;
char *cfg_lvsize;
char *cfg_sw;
char *cfg_hierarchy;
char *cfg_policy;
struct list_head g_caches;

extern cache_operations_t cache_inclusive;

static int load_cfg()
{
    /* read configuration from user settins */
    /*
      ## define hardware arch
      ## 1. i386, 2. x86_64, 3. arm
      arch=1
      ## define cache type
      ## ICache or DCache
      type = 0
      ## define the cache level
      level = 3 
      ## define every cache level's size, 32K, 64K, 2M
      lvsize = 1,5,8
      ## define every cache line's size, 16B, 32B
      ## define cache set associative
      sw = 2
      ## This value could derived from cache size and ways
      sa = 
      ## define every cache level's hierarchy
      ## INCLUSIVE, NON-INCLUSIVE, EXCLUSIVE
      hierarchy = 0,1,2
      ## define every cache level's conservative policy
      policy = 1,2,2
     */
    struct cfg_line cfg[] = {
        {"arch", &cfg_arch, TYPE_INT, PARM_MAND, 0, 1},
        {"type", &cfg_type, TYPE_INT, PARM_MAND, 0, 1},
        {"level", &cfg_level, TYPE_INT, PARM_MAND, 0, 3},
        {"linesize", &cfg_linesize, TYPE_INT, PARM_MAND, 0, 64},
        {"lvsize", &cfg_lvsize, TYPE_STRING, PARM_MAND, 0, 0},
        {"sw", &cfg_sw, TYPE_STRING, PARM_MAND, 0, 0},
        {"hierarchy", &cfg_hierarchy, TYPE_STRING, PARM_MAND, 0, 0},
        {"policy", &cfg_policy, TYPE_STRING, PARM_MAND, 0, 0},
        {NULL, NULL, 0, 0, 0, 0}
    };

    parse_cfg_file(cfg_file, cfg, 1, 1);

    printf("\nuser settinngs:"                  \
           "\n\t1. arch: %d"                    \
           "\n\t1. type: %d"                    \
           "\n\t2. level: %d"                   \
           "\n\t3. linesize: %d"                \
           "\n\t4. lvsize: %s"                  \
           "\n\t5. sw: %s"                      \
           "\n\t6. hierarchy: %s"               \
           "\n\t7. policy: %s",                 \
           cfg_arch, cfg_type, cfg_level, cfg_linesize, cfg_lvsize,
           cfg_sw, cfg_hierarchy, cfg_policy);
}

int *get_cache_lvsize()
{
	int *lvsize = malloc(sizeof(int)*cfg_level);
	char *str_lvsize = cfg_lvsize;
	char *delim = ",", *token = NULL;
	int index = 0;

	token = strtok(str_lvsize, delim);
	while (token) {
		lvsize[index++]	 = atoi(token);
		token = strtok(NULL, delim);
	}

	return lvsize;
}

#define CACHE_LV_2_SIZE_K(index) ({int k = 1024; int size = 0;			\
						switch(index)                                           \
                {                                                   \
								case 1: size = 16*k;	 break;                       \
								case 2: size = 32*k;	break;                        \
								case 3: size = 64*k;	break;                        \
								case 4: size = 128*k;	break;                        \
								case 5: size = k*k;		break;                        \
								case 6: size = 2*k*k;	break;                        \
								case 7: size = 4*k*k;	break;                        \
								case 8: size = 8*k*k;	break;                        \
              	case 9: size = 16*k*k;	break;                      \
								default: size = 64*k;	break;                        \
                }                                                   \
						size;                                                   \
					})

int *get_cache_sways()
{
    int *sw = malloc(sizeof(int)*cfg_level);
    char *str_sw = cfg_sw;
    char *delim = ",", *token = NULL;
    int index = 0;

    token = strtok(str_sw, delim);
    while (token) {
        sw[index++]	 = atoi(token);
        token = strtok(NULL, delim);
    }

    return sw;
}

#define SET_WAYS_2_SETS(size, linesize, ways) ((size)/(linesize)/(ways))


int main()
{
    int cache_line = 0, linesize;
    int set_associatives = 0;
    int *lvsize = NULL;
    int *sw = NULL;
    int size = 0;
    int ways = 0;

    printf("<usage - first> sets the config files in conf/cfg.cache \n");
    INIT_LIST_HEAD(&g_caches);
    load_cfg();

    lvsize = get_cache_lvsize();
    sw = get_cache_sways();

    if (!lvsize || !sw) {
        puts("please adjust cfg.cache file about cache parameters");
        return -1;
    }

    linesize = cfg_linesize;

    for (int i = 0; i < cfg_level; ++i) {
        printf("\nlvsize is %d", lvsize[i]);
        size = CACHE_LV_2_SIZE_K(lvsize[i]);
        printf("\ncache size is %d", size);
        ways = sw[i];
        set_associatives = SET_WAYS_2_SETS(size, linesize, ways);
        cache_t *cache = malloc(sizeof(cache_t) + sizeof(cache_set_t*)*set_associatives);

        if (!cache) {
            puts("\n---out of memory---\n");
            return -1;
        }

        printf("\n---create level %d---\n", i + 1);
        INIT_LIST_HEAD(&cache->list);
        list_add(&g_caches, &cache->list);
        cache->t_cache = cfg_type;
        cache->l_cache = i;
        cache->hp_cache = H_inclusive;
        cache->cp_cache = CP_lru;
        cache->sw_cache = SW_4ways;
        cache->ops = &cache_inclusive;
        cache->statistical_hit = 0;
        cache->statistical_miss = 0;

        for (int j = 0; j < set_associatives; ++j) {
            cache->c_data[j] = (cache_set_t*)malloc(sizeof(cache_line_t)*ways);
        }
    }
    puts("init cache done");
    return 0;
}
