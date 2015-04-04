#include "cachesim.hpp"
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


cache_block_t **cache;
cache_block_t *victim_cache;
cache_block_t *dummy_cache;

uint64_t ncache_blocks;
uint64_t cache_block_size;
uint64_t cache_size;
uint64_t nvictim_cache_blocks;
uint64_t tag_mask;
uint64_t block_mask;
uint64_t nsets;
uint64_t timestamp =0;

uint64_t nb;
uint64_t nc;
uint64_t ns;

char blocking_policy;
char replacement_policy;

bool is_debug = false;

double get_hit_time();
double get_blocking_mpenalty();
double get_sblocking_mpenalty();

/**
 * Subroutine for initializing the cache. You many add and initialize any global or heap
 * variables as needed.
 * XXX: You're responsible for completing this routine
 *
 * @c The total number of bytes for data storage is 2^C
 * @b The size of a single cache line in bytes is 2^B
 * @s The number of blocks in each set is 2^S
 * @v The number of blocks in the victim cache is 2^V
 * @st The storage policy, BLOCKING or SUBBLOCKING (refer to project description for details)
 * @r The replacement policy, LRU or NMRU_FIFO (refer to project description for details)
 */
void setup_cache(uint64_t c, uint64_t b, uint64_t s, uint64_t v, char st, char r) {
	uint64_t i;
	if(is_debug){
		printf("setting up cache\n");	
	}	
	ncache_blocks = pow(2,s);
	cache_block_size = pow(2,b);
	nb = b;
	nc = c;
	ns = s;
	cache_size = pow(2,c);
	nvictim_cache_blocks = pow(2,v); 

	blocking_policy = st;
	replacement_policy = r;	

    tag_mask = pow(2, c-s)-1;
	block_mask = pow(2,b)-1; 

	// setting up cache strucutre	
	nsets = pow(2,(c-b-s));
	cache = (cache_block_t **)malloc(sizeof(cache_block_t *)*nsets);
	for(i=0; i < nsets; i++){
		cache[i] = (cache_block_t *)malloc(sizeof(cache_block_t)*ncache_blocks);	
	}
	// setting up victim cache
	victim_cache = (cache_block_t *)malloc(sizeof(cache_block_t)*nvictim_cache_blocks);
	dummy_cache = (cache_block_t *)malloc(sizeof(cache_block_t)*nvictim_cache_blocks);

	if(is_debug){
		printf("Done setting up cache...\n");	
		printf("nsets: %" PRIu64 "\n",nsets);
		printf("ncache_blocks: %" PRIu64 "\n",ncache_blocks);
		printf("tag_mask: %" PRIu64 "\n",tag_mask);
		printf("block_mask: %" PRIu64 "\n",block_mask);
	}	
}
    
uint64_t get_timestmp(){
	return timestamp++;
}

/*
	search through the victim cache and return the matchin cache block
	index if its a hit.
	it its a miss, return the LRU cache block
	return : 1 - victim cache hit
			 0 - cache miss 
*/
int finding_victim_cache(uint64_t address, uint64_t *indexp){
	uint64_t i;
	uint64_t lru=0;
	uint64_t tag  = address & (~block_mask);
	for(i=0; i < nvictim_cache_blocks ; i++){
		if(victim_cache[i].tag == tag){ // victim cache hit
			*indexp=i;
			 return 1;
		} 
		if(victim_cache[i].last_access_time < victim_cache[lru].last_access_time){
			lru = i;	
		}
	} 
	*indexp=lru;
	return 0;
}

int find_lru_block(cache_block_t * cache_set, uint64_t size){
	uint64_t i;
	uint64_t lru_index=0;
	for(i=0; i < size ;i++){
	    if(cache_set[i].last_access_time < cache_set[lru_index].last_access_time){
			lru_index=i;
		}
	}	
	return lru_index;
}


int find_nmru_fifo_block(cache_block_t *cache_set, uint64_t size){
   //first get the LRU block
   uint64_t lru_index = find_lru_block(cache_set,size);
   uint64_t i;
   uint64_t oldest_index=0;
   for(i=0;i<size;i++){
		if(i == lru_index){ //excluding the LRU block
			continue;
		}
		if(cache_set[i].brought_in_time < cache_set[oldest_index].brought_in_time){
			oldest_index = i;
		}
	}
	return oldest_index;
}

/**
 * Subroutine that simulates the cache one trace event at a time.
 * XXX: You're responsible for completing this routine
 *
 * @rw The type of event. Either READ or WRITE
 * @address  The target memory address
 * @p_stats Pointer to the statistics structure
 */
void cache_access(char rw, uint64_t address, cache_stats_t* p_stats) {
	uint64_t i; 
	uint64_t mc_index,vc_index;

	uint64_t tag = address >> (nc-ns);
	uint64_t set_index = (address & tag_mask) >> nb;
	uint64_t sblock_index = (address & block_mask) >> (nb-1);

	if(is_debug){
		printf("cache request: %" PRIu64 "\n",address);
		printf("main cache tag: %" PRIu64 "\n",tag);
		printf("set index: %" PRIu64 "\n",set_index);
		printf("sblock index: %" PRIu64 "\n\n",sblock_index);
	}
	assert(set_index >= 0 && set_index <nsets);

	// recording accesses, reads and writes
	p_stats->accesses++;
	if(rw == READ){
		p_stats->reads++;
	}else if(rw == WRITE){
		p_stats->writes++;
	} 
	
	for(i=0; i < ncache_blocks ; i++){
		if(cache[set_index][i].tag == tag){ //cache hit
			if(blocking_policy == 'B'){
				if(is_debug){
					printf("cache hit\n");
				}	
				cache[set_index][i].last_access_time = get_timestmp();
				return;
			}else if(blocking_policy == 'S'){
				//check if the sub block is valid as well
				if(cache[set_index][i].v[sblock_index]){
					if(is_debug){
						printf("cache hit\n");
					}	
					// sub block hit
					cache[set_index][i].last_access_time = get_timestmp();
					return;
				} 
				break;
			}
		}
	}	 	
    // at this point main cache miss has happened
	if(rw == READ){
		p_stats->read_misses++;
		p_stats->misses++;
	}else if(rw == WRITE){
		p_stats->write_misses++;
		p_stats->misses++;
	} 
	if(is_debug){
		printf("cache miss\n");
	}	

	if(replacement_policy == 'L'){
		mc_index = find_lru_block(cache[set_index],ncache_blocks);	
		assert(mc_index >=0 && mc_index < ncache_blocks);
	}else if(replacement_policy == 'N'){ 
		mc_index = find_nmru_fifo_block(cache[set_index],ncache_blocks);
		assert(mc_index >=0 && mc_index < ncache_blocks);
	}	

    int found = finding_victim_cache(address,&vc_index);
	assert(vc_index >=0 && vc_index < nvictim_cache_blocks);
	if(found){ //victim hit. swap the block
		if(is_debug){
			printf("found in victim cache\n");
		}	
		cache_block_t temp = cache[set_index][mc_index];

		cache[set_index][mc_index] = victim_cache[vc_index];
		cache[set_index][mc_index].last_access_time = get_timestmp();
		cache[set_index][mc_index].brought_in_time = get_timestmp();
		if(blocking_policy == 'B'){
			assert(cache[set_index][mc_index].v[0]);
			assert(cache[set_index][mc_index].v[1]);
		}else if(blocking_policy == 'S'){
			if(!cache[set_index][mc_index].v[sblock_index]){
				if(is_debug){
					printf("sub block not found, bringing in other half\n\n");
				}	
				//bringing in the other half	
				cache[set_index][mc_index].v[sblock_index] = true;
				if(rw == READ){
					p_stats->read_misses_combined++;
				}else if(rw == WRITE){
					p_stats->write_misses_combined++;
				} 
				assert(cache[set_index][mc_index].v[~sblock_index]);
			}
		}

		victim_cache[vc_index] = temp;
		victim_cache[vc_index].last_access_time = get_timestmp();
		
	}else{ //victim miss. index of LRU in victim cache

		if(rw == READ){
			p_stats->read_misses_combined++;
		}else if(rw == WRITE){
			p_stats->write_misses_combined++;
		} 

		cache_block_t temp = cache[set_index][mc_index];
		//bringing in new cache line
		cache[set_index][mc_index].last_access_time = get_timestmp();
		cache[set_index][mc_index].brought_in_time = get_timestmp();
		cache[set_index][mc_index].tag = tag;
		if(blocking_policy == 'B'){
			cache[set_index][mc_index].v[0] = true;
			cache[set_index][mc_index].v[1] = true;
		}else if(blocking_policy == 'S'){
			cache[set_index][mc_index].v[sblock_index] = true;
			cache[set_index][mc_index].v[~sblock_index] = false;
		}

		victim_cache[vc_index] = temp;
		victim_cache[vc_index].last_access_time = get_timestmp();
	}
	return;
}

/**
 * Subroutine for cleaning up any outstanding memory operations and calculating overall statistics
 * such as miss rate or average access time.
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void complete_cache(cache_stats_t *p_stats) {
// free memory	
	uint64_t i;
	for(i=0; i<nsets ; i++){
		free(cache[i]);
	}
	free(victim_cache);



}


double get_hit_time(){
	double hit_time = 0.2 * pow(2,ns);
	return hit_time;
}


double get_blocking_mpenalty(){
	double blocking_penalty = 0.2*pow(2,ns) + 50 + 0.25*pow(2,nb);
	return blocking_penalty;
}


double get_sblocking_mpenalty(){
	double sblocking_penalty = 0.2*pow(2,ns) + 50 + 0.25*pow(2,(nb-1));
	return sblocking_penalty;
}
