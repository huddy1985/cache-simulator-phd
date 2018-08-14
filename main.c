#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cfg.h"
#include "cache.h"

static char *cfg_file = "conf/cfg.cache";

cache_t g_cache;

/* for read the data from config file */
int cfg_type;
int cfg_level;
char *cfg_lvsize;
char *cfg_sw;
char *cfg_hierarchy;
char *cfg_policy;

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
        {"type", &cfg_type, TYPE_INT, PARM_MAND, 0, 0},
        {"level", &cfg_level, TYPE_INT, PARM_MAND, 0, 3},
        {"lvsize", &cfg_lvsize, TYPE_STRING, PARM_MAND, 0, 0},
        {"sw", &cfg_sw, TYPE_STRING, PARM_MAND, 0, 0},
        {"hierarchy", &cfg_hierarchy, TYPE_STRING, PARM_MAND, 0, 0},
        {"policy", &cfg_policy, TYPE_STRING, PARM_MAND, 0, 0},
        {NULL, NULL, 0, 0, 0, 0}
    };

}

int main()
{
    printf("<usage - first> sets the config files in conf/cfg.cache ");
    load_cfg();
    return 0;
}