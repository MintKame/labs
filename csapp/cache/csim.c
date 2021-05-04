/* simulate the cache mem */
#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>
#include <string.h>

// each line in the cache
typedef struct
{
    int valid;
    unsigned long tag;
    int times; // cnt for not use time, used for LRU
} Line;

void parse(char *trace);
void visit(unsigned long set, unsigned long tag);

int verbose = 0;                  // whether verbose mode
int set_bit, block_bit, tag_bit;  // addr's diff part's bit size
int line_size;                    // line size of cache's set
unsigned long set_mask, tag_mask; // used to get set and tag from addr

// count the total hit, miss, evict of access
int hit_cnt = 0, miss_cnt = 0, eviction_cnt = 0;
Line *cache; // the simulated cache

int main(int argc, char *argv[])
{
    char *file_name;
    // parses the command-line arguments
    int opt;
    while ((opt = getopt(argc, argv, "vs:E:b:t:")) != -1)
    {
        switch (opt)
        {
        case 'v':
            verbose = 1;
            break;
        case 's':
            set_bit = atoi(optarg);
            break;
        case 'E':
            line_size = atoi(optarg);
            break;
        case 'b':
            block_bit = atoi(optarg);
            break;
        case 't':
            file_name = optarg;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-v] -s <s> -E <E> -b <b> -t <tracefile>\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    tag_bit = 64 - set_bit - block_bit;
    // compute mask
    tag_mask = ((1 << tag_bit) - 1) << (64 - tag_bit);
    set_mask = ((1 << set_bit) - 1) << block_bit;

    // allocate space for the cache
    cache = (Line *)malloc(sizeof(Line) * (1 << set_bit) * line_size);
    // open the file and parse each trace
    FILE *fp = fopen(file_name, "r");
    char trace[1024] = {0}; // store each trace
    while (fgets(trace, sizeof(trace), fp) != NULL)
    {
        parse(trace);
    }
    fclose(fp);
    free(cache);
    printSummary(hit_cnt, miss_cnt, eviction_cnt);
    return 0;
}
/**
 * parse the trace from the file
 * @para: the string prased
 * 
*/
void parse(char *trace)
{
    // ignore instruction cache access
    if (trace[0] == 'I')
        return;
    // get addr from the trace
    // ignore request size (beacuse mem access not cross block boundaries)
    unsigned long addr;
    int tmp;
    sscanf(trace + 3, "%lx,%d", &addr, &tmp);
    // get the position in cache
    unsigned long set = (addr & set_mask) >> (block_bit);
    unsigned long tag = (addr & tag_mask) >> (64 - tag_bit);
    switch (trace[1])
    {
    case 'L': // load data
    case 'S': // store data
        visit(set, tag);
        break;
    case 'M': // modify data
        visit(set, tag);
        visit(set, tag);
        break;
    }
}

/**
 * visit the cache's block 
 * @para: set - the set block maped to 
 *        tag - tag's content stored in line
*/
void visit(unsigned long set, unsigned long tag)
{
    int base = set * line_size; // set's first line
    int free_index = -1, hit_pos = -1;
    int victim = base;
    // traverse every line in the set
    for (int i = base; i < base + line_size; i++)
    {

        if (cache[i].valid) // valid
        {
            if (cache[i].tag == tag) // hit
            {
                hit_pos = i;
                break;
            }
            else if (free_index < 0 && // the block least read
                     cache[i].times > cache[victim].times)
            {
                victim = i;
            }
        }
        else if (free_index < 0) // 1st free block
        {
            free_index = i;
        }
    }
    if (hit_pos != -1) // hit
    {
        hit_cnt++;
        cache[hit_pos].times = -1;
    }
    else // miss
    {
        miss_cnt++;
        if (free_index < 0) // conflict
        {
            eviction_cnt++;
            free_index = victim;
        }
        // cache the new block
        cache[free_index].valid = 1;
        cache[free_index].tag = tag;
        cache[free_index].times = -1;
    }

    // update times
    for (int i = base; i < base + line_size; i++)
    {
        cache[i].times++;
    }
}