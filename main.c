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
        {"type", &cfg_type, TYPE_INT, PARM_MAND, 0, 1},
        {"level", &cfg_level, TYPE_INT, PARM_MAND, 0, 3},
        {"lvsize", &cfg_lvsize, TYPE_STRING, PARM_MAND, 0, 0},
        {"sw", &cfg_sw, TYPE_STRING, PARM_MAND, 0, 0},
        {"hierarchy", &cfg_hierarchy, TYPE_STRING, PARM_MAND, 0, 0},
        {"policy", &cfg_policy, TYPE_STRING, PARM_MAND, 0, 0},
        {NULL, NULL, 0, 0, 0, 0}
    };

    parse_cfg_file(cfg_file, cfg, 1, 1);

    printf("\n\tuser settinngs:" \
           "\n\t\t1. type: %d" \
           "\n\t\t2. level: %d" \
           "\n\t\t3. lvsize: %s" \
           "\n\t\t4. sw: %s" \
           "\n\t\t5. hierarchy: %s" \
           "\n\t\t6. policy: %s",
           cfg_type, cfg_level, cfg_lvsize,
           cfg_sw, cfg_hierarchy, cfg_policy);
}

int main()
{
    int cache_line = 0;
    int set_associative = 0;

    printf("<usage - first> sets the config files in conf/cfg.cache \n");
    INIT_LIST_HEAD(&g_caches);
    load_cfg();



    for (int i = 0; i < cfg_level; ++i) {
        cache_t *cache = malloc(sizeof(cache_t) + cache_line);
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
        // cache->sa_cache = SA_set_4;
        cache->sw_cache = SW_4ways;
        cache->ops = &cache_inclusive;
        cache->statistical_hit = 0;
        cache->statistical_miss = 0;
    }
    puts("init cache done");
    return 0;
}
