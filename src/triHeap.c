
#include "triHeap.h"
#include "triMemory.h"
#include <string.h>

// A MEMORY BLOCK ENTRY IS MADE UP LIKE THAT:
// bit:  31     32    30 - 15    14-0
//		free   block    prev     size
//
// bit 31: free bit, indicating if block is allocated or not
// bit 30: blocked bit, indicating if block is part of a larger block (0) - used for error resilience
// bit 30-15: block index of previous block
// bit 14- 0: size of current block
//
// This management can handle a max amount of 2^15 = 32768 blocks, which resolves to 32MB at blocksize of 1024 bytes
//
#define __CBLOCK_GET_SIZE(x)    ((x & 0x7FFF))
#define __CBLOCK_GET_PREV(x)    ((x >> 15) & 0x7FFF)
#define __CBLOCK_GET_FREE(x)    ((x >> 31))
#define __CBLOCK_GET_BLOCK(x)   ((x >> 30) & 0x1)
#define __CBLOCK_SET_SIZE(x,y)  x=((x & ~0x7FFF) | ((y) & 0x7FFF))
#define __CBLOCK_ADD_SIZE(x,y)  x=((x & ~0x7FFF) | (((x & 0x7FFF)+((y) & 0x7FFF)) & 0x7FFF))
#define __CBLOCK_SET_PREV(x,y)  x=((x & ~0x3FFF8000) | (((y) & 0x7FFF)<<15))
#define __CBLOCK_SET_FREE(x,y)  x=((x & 0x7FFFFFFF) | (((y) & 0x1)<<31))
#define __CBLOCK_SET_BLOCK(x,y) x=((x & 0xBFFFFFFF) | (((y) & 0x1)<<30))
#define __CBLOCK_MAKE(s,p,f,n)   (((f & 0x1)<<31) | ((n & 0x1)<<30) | (((p) & 0x7FFF)<<15) | ((s) & 0x7FFF))
#define __CBLOCK_GET_FREEBLOCK(x) ((x>>30) & 0x3)		// returns 11 if block is a starting block and free, 10 if block is a starting block and allocated, 0x if it is a non-starting block (don't change)

#define __CBLOCKS(x,y) ((x+y-1)/y)
#define __CBLOCKSIZE(x,y) ((x+y-1)&~(y-1))




triHeap* triHeapCreate( void* base, triU32 size, triU32 block_size )
{
	triHeap* mem = triMalloc(sizeof(triHeap));
	if (mem==0) return(0);
	
	mem->_base = base;
	mem->_size = size;
	if (block_size==0)
		block_size = size >> 12;		// have 4096 blocks to manage at default
	if (block_size<16)
		block_size = 16;				// minimum block size of 16

	mem->_block_size = block_size;
	mem->_n_blocks = size/block_size;
	if (mem->_n_blocks>32768)
	{
		mem->_n_blocks = 32768;
		mem->_block_size = size >> 15;
	}
	mem->_free = mem->_n_blocks;
	mem->_largest_block = mem->_n_blocks;
	mem->_largest_update = 0;
	mem->_blocks = triMalloc(sizeof(triU32)*mem->_n_blocks);
	if (mem->_blocks==0)
	{
		triFree(mem);
		return(0);
	}
	memset( mem->_blocks,0,sizeof(triU32)*mem->_n_blocks);
	mem->_blocks[0] = __CBLOCK_MAKE(mem->_n_blocks,0,1,1);
	return(mem);
}

void triHeapDestroy( triHeap* mem )
{
	if (mem==0) return;
	triFree(mem->_blocks);
	triFree(mem);
	return;
}

static void __triHeapFindLargestBlock( triHeap* mem )
{
	if (mem==0) return;
	triS32 i = 0;
	mem->_largest_block = 0;
	triU32* blocks = mem->_blocks;
	while (i<mem->_n_blocks)
	{
		triS32 csize = __CBLOCK_GET_SIZE(blocks[i]);
		if (__CBLOCK_GET_FREEBLOCK(blocks[i])==3 && csize>mem->_largest_block)
			mem->_largest_block = csize;
		i += csize;
	}
	mem->_largest_update = 0;
}

#ifdef _DEBUG
void __triHeapwalk( triHeap* mem )
{
	if (mem==0) return;
	triS32 i = 0;
	while (i<mem->_n_blocks)
	{
		printf("BLOCK %i:\n", i);
		printf("  free: %i\n", __CBLOCK_GET_FREEBLOCK(mem->_blocks[i]));
		printf("  size: %i\n", __CBLOCK_GET_SIZE(mem->_blocks[i]));
		printf("  prev: %i\n", __CBLOCK_GET_PREV(mem->_blocks[i]));
		i+=__CBLOCK_GET_SIZE(mem->_blocks[i]);
	}
}
#endif

void* triHeapAlloc( triHeap* mem, triU32 size )
{
	if (mem==0) return(0);
	if (size==0) return(0);
	
	triS32 i = 0;
	triS32 j = 0;
	triS32 bsize = __CBLOCKS(size, mem->_block_size);
	
	if (mem->_largest_block<bsize)
	{
		#ifdef _DEBUG
		printf("Not enough memory to allocate %i bytes (largest: %i)!\n",size,triHeapLargestblock(mem));
		#endif
		return(0);
	}

	#ifdef _DEBUG
	printf("allocating %i bytes, in %i blocks\n", size, bsize);
	#endif
	triU32* blocks = mem->_blocks;
	// Find smallest block that still fits the requested size
	triS32 bestblock = -1;
	triS32 bestblock_prev = 0;
	triS32 bestblock_size = mem->_n_blocks+1;
	while (i<mem->_n_blocks)
	{
		triS32 csize = __CBLOCK_GET_SIZE(blocks[i]);
		if (__CBLOCK_GET_FREEBLOCK(blocks[i])==3 && csize>=bsize)
		{
			if (csize<bestblock_size)
			{
				bestblock = i;
				bestblock_prev = j;
				bestblock_size = csize;
			}
			
			if (csize==bsize)
				break;
		}
		j = i;
		i += csize;
	}
	
	if (bestblock<0)
	{
		#ifdef _DEBUG
		printf("Not enough memory to allocate %i bytes (largest: %i)!\n",size,triHeapLargestblock(mem));
		#endif
		return(0);
	}
	
	i = bestblock;
	j = bestblock_prev;	
	triS32 csize = bestblock_size;
	blocks[i] = __CBLOCK_MAKE(bsize,j,0,1);
	
	triS32 next = i+bsize;
	if (csize>bsize && next<mem->_n_blocks)
	{
		blocks[next] = __CBLOCK_MAKE(csize-bsize,i,1,1);
		triS32 nextnext = i+csize;
		if (nextnext<mem->_n_blocks)
		{
			__CBLOCK_SET_PREV(blocks[nextnext], next);
		}
	}

	mem->_free -= bsize;
	if (mem->_largest_block==csize)		// if we just allocated from one of the largest blocks
	{
		if ((csize-bsize)>(mem->_free/2))
			mem->_largest_block = (csize-bsize);		// there can't be another largest block
		else
			mem->_largest_update = 1;
	}
	return ((void*)(mem->_base + (i*mem->_block_size)));
}


void* triHeapCalloc( triHeap* mem, triU32 size )
{
	void* ptr = triHeapAlloc( mem, size );
	
	if (ptr!=0)
		memset( ptr, 0, size );
		
	return ptr;
}


void* triHeapRealloc( triHeap* mem, void* ptr, triU32 size )
{
	if (mem==0) return(0);
	if (size==0)
	{
		triHeapFree( mem, ptr );
		return(0);
	}
	if (ptr==0) return triHeapAlloc( mem, size );

	triS32 i = 0;
	triS32 j = 0;
	triS32 bsize = __CBLOCKS(size, mem->_block_size);
	
	triU32* blocks = mem->_blocks;
	
	triS32 block = ((triU32)ptr - (triU32)mem->_base)/mem->_block_size;
	if (block<0 || block>mem->_n_blocks)
	{
		#ifdef _DEBUG
		printf("Block is out of range: %i (0x%x)\n", block, (triU32)ptr);
		#endif
		return(ptr);
	}
	
	triS32 csize = __CBLOCK_GET_SIZE(blocks[block]);
	#ifdef _DEBUG
	printf("reallocating block %i (0x%x), size: %i, new size: %i\n", block, (triU32)ptr, csize, bsize);
	#endif

	if (__CBLOCK_GET_FREEBLOCK(blocks[block])!=1 || csize==0)
	{
		#ifdef _DEBUG
		printf("Block was not allocated!\n");
		#endif
		return(ptr);
	}
	
	// Nothing to do
	if (bsize==csize) return(ptr);
	
	if (bsize>csize+mem->_free) return(0);	// User wants more than is possible
	
	triS32 next = block+csize;
	triS32 nsize = (next<mem->_n_blocks)?__CBLOCK_GET_SIZE(blocks[next]):0;
	triS32 nextnext = next + nsize;
	
	// Just shrink the block
	if (bsize<csize)
	{
		__CBLOCK_SET_SIZE(blocks[block], bsize);	// Reset the blocks size
		triS32 diff = csize - bsize;
		mem->_free += diff;
		triS32 newsize = diff;
		if (next<mem->_n_blocks && __CBLOCK_GET_FREEBLOCK(blocks[next])==3)
		{
			newsize += nsize;
			__CBLOCK_SET_BLOCK(blocks[next],0);	// Mark next block as intra block
			next = nextnext;
		}
		
		blocks[block+bsize] = __CBLOCK_MAKE(newsize,block,1,1);
		if (mem->_largest_block < newsize)
		{
			mem->_largest_block = newsize;
			mem->_largest_update = 0;		// No update necessary any more, because update only necessary when largest has shrinked at most
		}
		if (next<mem->_n_blocks && next<block+bsize)
			__CBLOCK_SET_PREV(blocks[next],block+bsize);
		return(ptr);
	}
	
	
	triS32 newsize = csize - bsize;
	triU32 nextfree = (next<mem->_n_blocks && __CBLOCK_GET_FREEBLOCK(blocks[next])==3);
	if (nextfree)
		newsize += nsize;
	
	triS32 prev = __CBLOCK_GET_PREV(blocks[block]);
	triS32 psize = (prev<block)?__CBLOCK_GET_SIZE(blocks[prev]):0;
	triU32 prevfree = (prev<block && __CBLOCK_GET_FREEBLOCK(blocks[prev])==3);
	if (prevfree)
		newsize += psize;


	// We do this because realloc spec says that the ptr may not be changed if we cannot realloc
	if (newsize<0)
	{
		// We can't fit the new size within the next and/or previous blocks,
		// so we have to find another block that is large enough
		void* newptr = triHeapAlloc( mem, size );
		if (newptr==0) return(0);
		
		memcpy( newptr, ptr, csize*mem->_block_size );
		triHeapFree( mem, ptr );
		return(newptr);
	}
	
	newsize = csize - bsize;
	// Next block is free
	if (nextfree)
	{
		newsize += nsize;
				
		// Merge next block with current
		__CBLOCK_SET_SIZE(blocks[block], csize+nsize);	// Reset the blocks size
		__CBLOCK_SET_BLOCK(blocks[next],0);	// Mark next block as intra block
		if (nextnext<mem->_n_blocks)
			__CBLOCK_SET_PREV(blocks[nextnext], block);
		next = nextnext;
		
		// We can fit the increase within the next block
		if (newsize >= 0)
		{
			mem->_free -= (nsize-newsize);
			__CBLOCK_SET_SIZE(blocks[block], bsize);	// Reset the blocks size
			next = block;
			if (newsize > 0)
			{
				// Create a new free block
				next += bsize;
				blocks[next] = __CBLOCK_MAKE(newsize,block,1,1);
			}
			if (nextnext<mem->_n_blocks)
				__CBLOCK_SET_PREV(blocks[nextnext], next);

			return (ptr);
		}
	}
	
	
	// Previous block is free
	if (prevfree)
	{
		newsize += psize;
		
		// Merge previous block with current
		__CBLOCK_ADD_SIZE(blocks[prev], __CBLOCK_GET_SIZE(blocks[block]));
		__CBLOCK_SET_BLOCK(blocks[block],0);	// mark current block as inter block
		if (next<mem->_n_blocks)
			__CBLOCK_SET_PREV(blocks[next],prev);
		
		// We can fit the increase within the previous block?
		if (newsize >= 0)
		{
			void* newptr = (void*)(mem->_base + (prev*mem->_block_size));
			memcpy( newptr, ptr, csize*mem->_block_size );
			
			__CBLOCK_SET_SIZE(blocks[prev], bsize);
			__CBLOCK_SET_FREE(blocks[prev],0);	// Mark prev block as allocated
			
			next = prev;
			if (newsize > 0)
			{
				next += bsize;
				blocks[next] = __CBLOCK_MAKE(newsize,prev,1,1);
			}
			if (nextnext<mem->_n_blocks)
				__CBLOCK_SET_PREV(blocks[nextnext],next);
			
			return(newptr);
		}
	}
	
	// This should never happen
	return (0);
}



void triHeapFree( triHeap* mem, void* ptr )
{
	if (ptr==0) return;
	if (mem==0) return;
	
	triS32 block = ((triU32)ptr - (triU32)mem->_base)/mem->_block_size;
	if (block<0 || block>mem->_n_blocks)
	{
		#ifdef _DEBUG
		printf("Block is out of range: %i (0x%x)\n", block, (triU32)ptr);
		#endif
		return;
	}
	
	triU32* blocks = mem->_blocks;
	triS32 csize = __CBLOCK_GET_SIZE(blocks[block]);
	#ifdef _DEBUG
	printf("freeing block %i (0x%x), size: %i\n", block, (triU32)ptr, csize);
	#endif

	if (__CBLOCK_GET_FREEBLOCK(blocks[block])!=1 || csize==0)
	{
		#ifdef _DEBUG
		printf("Block was not allocated!\n");
		#endif
		return;
	}
	
	// Mark block as free
	__CBLOCK_SET_FREE(blocks[block],1);
	mem->_free += csize;
	
	triS32 next = block+csize;
	// Merge with previous block if possible
	triS32 prev = __CBLOCK_GET_PREV(blocks[block]);
	if (prev<block)
	{
		if (__CBLOCK_GET_FREEBLOCK(blocks[prev])==3)
		{
			__CBLOCK_ADD_SIZE(blocks[prev], csize);
			__CBLOCK_SET_BLOCK(blocks[block],0);	// mark current block as inter block
			if (next<mem->_n_blocks)
				__CBLOCK_SET_PREV(blocks[next], prev);
			block = prev;
		}
	}

	// Merge with next block if possible
	if (next<mem->_n_blocks)
	{
		if (__CBLOCK_GET_FREEBLOCK(blocks[next])==3)
		{
			__CBLOCK_ADD_SIZE(blocks[block], __CBLOCK_GET_SIZE(blocks[next]));
			__CBLOCK_SET_BLOCK(blocks[next],0);	// mark next block as inter block
			triS32 nextnext = next + __CBLOCK_GET_SIZE(blocks[next]);
			if (nextnext<mem->_n_blocks)
				__CBLOCK_SET_PREV(blocks[nextnext], block);
		}
	}

	// Update if a new largest block emerged
	if (mem->_largest_block<__CBLOCK_GET_SIZE(blocks[block])) 
	{
		mem->_largest_block = __CBLOCK_GET_SIZE(blocks[block]);
		mem->_largest_update = 0;		// No update necessary any more, because update only necessary when largest has shrinked at most
	}
}


void triHeapFreeAll( triHeap* mem )
{
	if (mem==0) return;
	memset( mem->_blocks,0,sizeof(triU32)*mem->_n_blocks);
	mem->_blocks[0] = __CBLOCK_MAKE(mem->_n_blocks,0,1,1);
}


triU32 triHeapMemavail( triHeap* mem )
{
	if (mem==0) return(0);
	return mem->_free * mem->_block_size;
}


triU32 triHeapLargestblock( triHeap* mem )
{
	if (mem==0) return(0);
	if (mem->_largest_update) __triHeapFindLargestBlock( mem );
	return mem->_largest_block * mem->_block_size;
}
