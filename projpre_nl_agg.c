/***************************************************************************************************************** 
File name: projpre_nl_agg.c

Descrption: This program simulates the behaviour of an aggressive prefetcher
******************************************************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>


//Eight way shared cache parameters
#define SETS_IN_EIGHT_WAY 1024//Enter sets in a eight way cache(total cache size/block size = no of blocks, sets = no of blocks/4)
#define SET_INDEX 10//Enter the index bits required to address these sets(2^(index bits) = sets in cache)
#define SET_MASK 0x3ff//Enter the mask to find Index bits(mask of 1's of length of index bits)
#define SET_BLOCK_SIZE 8//Enter block size(in bytes)
#define SET_OFFSET 3//Enter offset bits(2^(offset) = block size)
#define SET_TAG_SHIFT 13//Enter tag shift required to find tag(SET_INDEX+SET_OFFSET)



//Fully asscociative shared cache parameters
#define FULL_BLOCKS_IN_CACHE 8192//Enter the blocks in fully associative cache(total cache size/block size)
#define FULL_BLOCK_SIZE 8//Enter block size(in bytes)
#define FULL_OFFSET 3//Enter offset bits(2^(offset) = block size)
#define FULL_TAG_SHIFT 3//Enter tag shift required to find tag(SET_OFFSET)


//prefetch spec
#define PRE_BLOCKS 512






//8-way associative cache structure
struct eightway_cache{
	uint64_t sets[SETS_IN_EIGHT_WAY][8];
	uint64_t no_of_access[SETS_IN_EIGHT_WAY][8];
	int no_of_hits;
	int no_of_misses;
};


struct full_cache{ 
	uint64_t blocks[FULL_BLOCKS_IN_CACHE];
	uint64_t no_of_access[FULL_BLOCKS_IN_CACHE]; 
	int no_of_hits;
	int no_of_misses;
};


uint64_t hexaddresstobits(char hex_address[]);
int findoperation(char hex_address[]);

void read_from_eight_way_cache(struct eightway_cache *cache, uint64_t address_in_trace);
void read_from_full_cache(struct full_cache *cache, uint64_t address_in_trace);
void write_to_eight_way_on_a_miss(struct eightway_cache *cache, uint64_t set, uint64_t tag);
void write_to_full_on_a_miss(struct full_cache *cache, uint64_t tag);

void write_to_eight_way_cache(struct eightway_cache *cache, uint64_t address_in_trace);
void write_to_full_cache(struct full_cache *cache, uint64_t address_in_trace);
void eight_next_line_prefetch(struct eightway_cache *cache, uint64_t address_in_trace);
void full_next_line_prefetch(struct full_cache *cache, uint64_t address_in_trace);

//no of cache reads
int reads = 0;

//no of cache writes
int writes = 0;

/*****************************************************************************************************************
Function: main()
Description: 
1. Initializes cache blocks
2. Reads the trace file
3. After determining the operation it reads/writes the cache with respect to operation
*******************************************************************************************************************/

int main(int argc, char *argv[]) 
{	

printf("Reminder to read readme file and change the cache parameters before compiling\n");
	
	char* tracefile = argv[2];
	char is_it_eightway[20];

	char is_it_full[10];
	strcpy(is_it_eightway, "eightway_cache");

	strcpy(is_it_full, "fully");
	struct eightway_cache assoccache;

	struct full_cache fullcache;
	assoccache.no_of_hits = 0;
	assoccache.no_of_misses = 0;

	fullcache.no_of_hits = 0;
	fullcache.no_of_misses = 0;


	for(int x =0; x<SETS_IN_EIGHT_WAY; x++){
		for(int y = 0; y<8; y++){
			assoccache.sets[x][y] = 0;
			assoccache.no_of_access[x][y] = 0;
		}
	}
	for(int i =0; i<FULL_BLOCKS_IN_CACHE; i++){
		fullcache.blocks[i] = 0;
		fullcache.no_of_access[i] = 0;
	}

	char line_of_trace[20];
   	FILE *pointer_to_line_in_trace;
	int operation_in_trace;

   	pointer_to_line_in_trace = fopen(tracefile , "r");
   	
	while((fgets(line_of_trace, 20, pointer_to_line_in_trace)!= NULL))
	{

		operation_in_trace = findoperation(line_of_trace);
//printf("%d\n", operation_in_trace);

		if(operation_in_trace == 87)
		{

			uint64_t address = hexaddresstobits(line_of_trace);
			if(strncmp(argv[1], is_it_eightway, 24)==0){
				write_to_eight_way_cache(&assoccache, address);
			}
			
			if(strncmp(argv[1], is_it_full, 19)==0){
				write_to_full_cache(&fullcache, address);
			}
		}
		else
		{

			uint64_t address = hexaddresstobits(line_of_trace);
			if(strncmp(argv[1], is_it_eightway, 24)==0){
				read_from_eight_way_cache(&assoccache, address);
				eight_next_line_prefetch(&assoccache, address);
				
			}
			
			if(strncmp(argv[1], is_it_full, 19)==0){
				read_from_full_cache(&fullcache, address);
				full_next_line_prefetch(&fullcache, address);
			}
		}
	}

	fclose(pointer_to_line_in_trace);


	if(strncmp(argv[1], is_it_eightway, 24)==0){

		printf("You selected a shared cache with 8-way associativity\n");

		printf("No of cache Hits: %d\n", assoccache.no_of_hits);
		printf("No of cache Misses: %d\n", assoccache.no_of_misses);

		printf("Cache hit rate: %2.4f\n", (double)assoccache.no_of_hits/(assoccache.no_of_hits+assoccache.no_of_misses));
		printf("Cache miss rate: %2.4f\n", (double)assoccache.no_of_misses/(assoccache.no_of_hits+assoccache.no_of_misses));

		printf("cache reads : %d\n", reads);
		printf("cache writes : %d\n", writes);
	}
	
	if(strncmp(argv[1], is_it_full, 19)==0){

		printf("You selected a shared cache with full associativity\n");

		printf("No of cache Hits: %d\n", fullcache.no_of_hits);
		printf("No of cache Misses: %d\n", fullcache.no_of_misses);

		printf("Cache hit rate: %2.4f\n", (double)fullcache.no_of_hits/(fullcache.no_of_hits+fullcache.no_of_misses));
		printf("Cache miss rate: %2.4f\n", (double)fullcache.no_of_misses/(fullcache.no_of_hits+fullcache.no_of_misses));

		printf("cache reads : %d\n", reads);
		printf("cache writes : %d\n", writes);
	}


}

/************************************************************************************************************************************
Function: hexaddresstobits(char)
Input: Hex address
Output: binary representation
Description: Coverts hexadecimal address to its binary representation
*************************************************************************************************************************************/

uint64_t hexaddresstobits(char hex_address[]) 
{
	uint64_t bit_level_rep = 0;
	int i = 4;
	while(hex_address[i]!= '\n'){
   		if (hex_address[i]<= '9' && hex_address[i]>='0'){
   			bit_level_rep = (bit_level_rep*16) + (hex_address[i] - '0');
   		}else{
   			if(hex_address[i] == 'a'){
   		 		bit_level_rep = (bit_level_rep*16) + 10;
   		 	}
   		 	if(hex_address[i] == 'b'){
   		 		bit_level_rep = (bit_level_rep*16) + 11;
   		 	}
   		 	if(hex_address[i] == 'c'){
   		 		bit_level_rep = (bit_level_rep*16) + 12;
   		 	}
   		 	if(hex_address[i] == 'd'){
   		 		bit_level_rep = (bit_level_rep*16) + 13;
   		 	}
   		 	if(hex_address[i] == 'e'){
   		 		bit_level_rep = (bit_level_rep*16) + 14;
   		 	}
   		 	if(hex_address[i] == 'f'){
   		 		bit_level_rep = (bit_level_rep*16) + 15;
   		 	}
   		}
   	    i++;
   	}

//printf("%llu\n", bit_level_rep);
    return bit_level_rep;
}



/*****************************************************************************************************************************************
Function: read_from_eight_way_cache(struct*, uint64_t)
Input: Structure pointer to cache and address read in trace file
Description: Performs cache read operation from eight way set associative shared cache. If the tag is present in cache block, it increments 
hits else it increments cache misses. And updates the current tag to the cache block
******************************************************************************************************************************************/

void read_from_eight_way_cache(struct eightway_cache *cache, uint64_t address_in_trace)
{
	uint64_t mask = SET_MASK;
	uint64_t identify_set = (address_in_trace >> SET_OFFSET) & mask;
	uint64_t tag = address_in_trace >> SET_TAG_SHIFT;

	for(int i = 0; i <8; i++){
		if(cache->sets[identify_set][i] == tag){
			cache->no_of_hits += 1;
			cache->no_of_access[identify_set][i] += 1;
			reads++;
			return;
		}	
	}
	writes++;
	cache->no_of_misses += 1;
	write_to_eight_way_on_a_miss(cache, identify_set, tag);

}

/*****************************************************************************************************************************************
Function: read_from_full_cache(struct*, uint64_t)
Input: Structure pointer to cache and address read in trace file
Description: Performs cache read operation from fully associative shared cache. If the tag is present in cache block, it increments 
hits else it increments cache misses. And updates the current tag to the cache block
******************************************************************************************************************************************/

void read_from_full_cache(struct full_cache *cache, uint64_t address_in_trace)
{
	uint64_t tag = address_in_trace >> FULL_OFFSET;

	for(int i = 0; i < FULL_BLOCKS_IN_CACHE; i++){
		if(cache->blocks[i] == tag){
			cache->no_of_hits += 1;
			cache->no_of_access[i] += 1;
			reads++;
			return;
		}	
	}
	writes++;
	cache->no_of_misses += 1;
	write_to_full_on_a_miss(cache, tag);

}

/************************************************************************************************************************************
Function: write_to_eight_way_on_a_miss(struct*, uint64_t, uint64_t)
Input: Structure pointer to cache, set to which write to be done and tag to be updated to the block
Description: Performs cache write operation to eight way set associative shared cache on a miss. Finds the least recently used cache 
block from the set and updates the tag to that cache block.
*************************************************************************************************************************************/

void write_to_eight_way_on_a_miss(struct eightway_cache *cache, uint64_t set, uint64_t tag)
{
	int least_recently_used = cache->no_of_access[set][0];
	for(int x = 0; x<8; x++){
		if(cache->no_of_access[set][x] < least_recently_used){
			least_recently_used = cache->no_of_access[set][x];
		}
	}
	
	for(int x = 0; x<8; x++){
		if(cache->no_of_access[set][x] == least_recently_used){
			cache->sets[set][x] = tag;
			cache->no_of_access[set][x] = 0;
			return;
		}
	}
}

/************************************************************************************************************************************
Function: write_to_eight_way_on_a_miss(struct*, uint64_t, uint64_t)
Input: Structure pointer to cache, set to which write to be done and tag to be updated to the block
Description: Performs cache write operation to eight way set associative shared cache on a miss. Finds the least recently used cache 
block from the set and updates the tag to that cache block.
*************************************************************************************************************************************/

void write_to_full_on_a_miss(struct full_cache *cache, uint64_t tag)
{
	int least_recently_used = cache->no_of_access[0];
	for(int x = 0; x < FULL_BLOCKS_IN_CACHE; x++){
		if(cache->no_of_access[x] < least_recently_used){
			least_recently_used = cache->no_of_access[x];
		}
	}
	
	for(int x = 0; x<FULL_BLOCKS_IN_CACHE; x++){
		if(cache->no_of_access[x] == least_recently_used){
			cache->blocks[x] = tag;
			cache->no_of_access[x] = 0;
			return;
		}
	}
}




/************************************************************************************************************************************
Function: write_to_eight_way_cache(struct*, uint64_t)
Input: Structure pointer to cache and address read in trace file
Description: Performs cache write operation to eight way set associative shared cache. Calls the function write_to_eight_way_on_a_miss to
find the least recently used cache block and updates it with tag
*************************************************************************************************************************************/

void write_to_eight_way_cache(struct eightway_cache *cache, uint64_t address_in_trace)
{
	

	uint64_t mask = SET_MASK;
	uint64_t identify_set = (address_in_trace >> SET_OFFSET) & mask;
	uint64_t tag = address_in_trace >> SET_TAG_SHIFT;
	writes++;
	write_to_eight_way_on_a_miss(cache, identify_set, tag);

}

/************************************************************************************************************************************
Function: write_to_eight_way_cache(struct*, uint64_t)
Input: Structure pointer to cache and address read in trace file
Description: Performs cache write operation to eight way set associative shared cache. Calls the function write_to_eight_way_on_a_miss to
find the least recently used cache block and updates it with tag
*************************************************************************************************************************************/

void write_to_full_cache(struct full_cache *cache, uint64_t address_in_trace)
{
	
	uint64_t tag = address_in_trace >> FULL_OFFSET;
	writes++;
	write_to_full_on_a_miss(cache, tag);

}

/************************************************************************************************************************************
Function: findoperation(char hex_address[]) 
Input: Pointer to the hex address array
Description: Finds the opration to be performed on an address. Possible operations in the trace file are data read, data write and 
instruction fetch
*************************************************************************************************************************************/

int findoperation(char hex_address[]) 
{
	int operation_in_trace = hex_address[0];
	return operation_in_trace;
}


void eight_next_line_prefetch(struct eightway_cache *cache, uint64_t address_in_trace)
{

uint64_t next_line1,next_line2, next_line3, next_line4, next_line5;
next_line1 = address_in_trace + 8;
next_line2 = address_in_trace + 16;
next_line3 = address_in_trace + 24;

uint64_t mask = SET_MASK;
	uint64_t identify_set1 = (next_line1 >> SET_OFFSET) & mask;
	uint64_t identify_set2 = (next_line2 >> SET_OFFSET) & mask;
	uint64_t identify_set3 = (next_line3 >> SET_OFFSET) & mask;
	
	uint64_t tag1 = next_line1 >> SET_TAG_SHIFT;
	uint64_t tag2 = next_line2 >> SET_TAG_SHIFT;
	uint64_t tag3 = next_line3 >> SET_TAG_SHIFT;
	

write_to_eight_way_on_a_miss(cache, identify_set1, tag1);
write_to_eight_way_on_a_miss(cache, identify_set2, tag2);
write_to_eight_way_on_a_miss(cache, identify_set3, tag3);

}


void full_next_line_prefetch(struct full_cache *cache, uint64_t address_in_trace)
{
uint64_t next_line;
next_line = address_in_trace + 8;
uint64_t tag = next_line >> FULL_OFFSET;
	writes++;
	write_to_full_on_a_miss(cache, tag);
}



