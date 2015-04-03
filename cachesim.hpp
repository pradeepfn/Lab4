#ifndef CACHESIM_HPP
#define CACHESIM_HPP

#include <cinttypes>

struct cache_stats_t {
    uint64_t accesses;
    uint64_t reads;
    uint64_t read_misses;
    uint64_t read_misses_combined;
    uint64_t writes;
    uint64_t write_misses;
    uint64_t write_misses_combined;
    uint64_t misses;
    uint64_t hit_time;
    uint64_t miss_penalty;
    double   miss_rate;
    double   avg_access_time;
    uint64_t storage_overhead;
    double   storage_overhead_ratio;
};

typedef struct cache_block_{
    uint64_t last_access_time;
	uint64_t tag;
    bool v0;
	bool v1;
	bool dirty;
} cache_block_t; 

void cache_access(char rw, uint64_t address, cache_stats_t* p_stats);
void setup_cache(uint64_t c, uint64_t b, uint64_t s, uint64_t v, char st, char r);
void complete_cache(cache_stats_t *p_stats);

static const uint64_t DEFAULT_C = 15;   /* 32KB Cache */
static const uint64_t DEFAULT_B = 5;    /* 32-byte blocks */
static const uint64_t DEFAULT_S = 3;    /* 8 blocks per set */
static const uint64_t DEFAULT_V = 2;    /* 4 victim blocks */

static const char     BLOCKING = 'B';
static const char     SUBBLOCKING = 'S';
static const char     DEFAULT_ST = BLOCKING;

static const char     LRU = 'L';
static const char     NMRU_FIFO = 'N';
static const char     DEFAULT_R = LRU;

/** Argument to cache_access rw. Indicates a load */
static const char     READ = 'r';
/** Argument to cache_access rw. Indicates a store */
static const char     WRITE = 'w';

#endif /* CACHESIM_HPP */
