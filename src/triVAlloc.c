/*
 * triVAlloc.h: Code for VRAM allocation routines
 * This file is part of the "tri Engine".
 *
 * Copyright (C) 2007 tri
 * Copyright (C) 2007 Alexander Berl 'Raphael' <raphael@fx-world.org>
 *
 * $Id: $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
 
#include "triVAlloc.h"
#include "triLog.h"


// Configure the memory to be managed
#define __MEM_SIZE 0x00200000
#define __MEM_START 0x04000000

// Configure the block size the memory gets subdivided into (page size)
// __MEM_SIZE/__BLOCK_SIZE may not exceed 2^15 = 32768
// The block size also defines the alignment of allocations
// Larger block sizes perform better, because the blocktable is smaller and therefore fits better into cache
// however the overhead is also bigger and more memory is wasted
#define __BLOCK_SIZE 256
#define __MEM_BLOCKS (__MEM_SIZE/__BLOCK_SIZE)
#define __BLOCKS(x) ((x+__BLOCK_SIZE-1)/__BLOCK_SIZE)
#define __BLOCKSIZE(x) ((x+__BLOCK_SIZE-1)&~(__BLOCK_SIZE-1))


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
#define __BLOCK_GET_SIZE(x)    ((x & 0x7FFF))
#define __BLOCK_GET_PREV(x)    ((x >> 15) & 0x7FFF)
#define __BLOCK_GET_FREE(x)    ((x >> 31))
#define __BLOCK_GET_BLOCK(x)   ((x >> 30) & 0x1)
#define __BLOCK_SET_SIZE(x,y)  x=((x & ~0x7FFF) | ((y) & 0x7FFF))
#define __BLOCK_ADD_SIZE(x,y)  x=((x & ~0x7FFF) | (((x & 0x7FFF)+((y) & 0x7FFF)) & 0x7FFF))
#define __BLOCK_SET_PREV(x,y)  x=((x & ~0x3FFF8000) | (((y) & 0x7FFF)<<15))
#define __BLOCK_SET_FREE(x,y)  x=((x & 0x7FFFFFFF) | (((y) & 0x1)<<31))
#define __BLOCK_SET_BLOCK(x,y) x=((x & 0xBFFFFFFF) | (((y) & 0x1)<<30))
#define __BLOCK_MAKE(s,p,f,n)   (((f & 0x1)<<31) | ((n & 0x1)<<30) | (((p) & 0x7FFF)<<15) | ((s) & 0x7FFF))
#define __BLOCK_GET_FREEBLOCK(x) ((x>>30) & 0x3)		// returns 11b if block is a starting block and free, 10b if block is a starting block and allocated, 0xb if it is a non-starting block (don't change)
#define __BLOCK0 ((__MEM_BLOCKS) | (1<<31) | (1<<30))


triU32	__mem_blocks[__MEM_BLOCKS] = { 0 };


static triS32 __largest_update = 0;
static triS32 __largest_block = __MEM_BLOCKS;
static triS32 __mem_free = __MEM_BLOCKS;


inline triVoid* vrelptr( triVoid *ptr )
{
	return (triVoid*)((triU32)ptr & ~__MEM_START);
}

inline triVoid* vabsptr( triVoid *ptr )
{
	return (triVoid*)((triU32)ptr | __MEM_START);
}


static triVoid __find_largest_block()
{
	triS32 i = 0;
	__largest_block = 0;
	while (i<__MEM_BLOCKS)
	{
		triS32 csize = __BLOCK_GET_SIZE(__mem_blocks[i]);
		if (__BLOCK_GET_FREEBLOCK(__mem_blocks[i])==3 && csize>__largest_block)
			__largest_block = csize;
		i += csize;
	}
	__largest_update = 0;
}

#ifdef _DEBUG
triVoid __memwalk()
{
	triS32 i = 0;
	if (__mem_blocks[0]==0) __mem_blocks[0] = __BLOCK0;
	while (i<__MEM_BLOCKS)
	{
		triLogPrint("BLOCK %i:\n", i);
		triLogPrint("  free: %i\n", __BLOCK_GET_FREEBLOCK(__mem_blocks[i]));
		triLogPrint("  size: %i\n", __BLOCK_GET_SIZE(__mem_blocks[i]));
		triLogPrint("  prev: %i\n", __BLOCK_GET_PREV(__mem_blocks[i]));
		i+=__BLOCK_GET_SIZE(__mem_blocks[i]);
	}
}
#endif

triVoid* triVAlloc( triU32 size )
{
	// Initialize memory block, if not yet done
	if (__mem_blocks[0]==0) __mem_blocks[0] = __BLOCK0;
	
	if (size==0) return(0);
	
	triS32 i = 0;
	triS32 j = 0;
	triS32 bsize = __BLOCKS(size);
	
	if (__largest_update==0 && __largest_block<bsize)
	{
		#ifdef _DEBUG
		triLogPrint("Not enough memory to allocate %i bytes (largest: %i)!\n",size,triVLargestblock());
		#endif
		return(0);
	}

	#ifdef _DEBUG
	triLogPrint("allocating %i bytes, in %i blocks\n", size, bsize);
	#endif
	// Find smallest block that still fits the requested size
	triS32 bestblock = -1;
	triS32 bestblock_prev = 0;
	triS32 bestblock_size = __MEM_BLOCKS+1;
	while (i<__MEM_BLOCKS)
	{
		triS32 csize = __BLOCK_GET_SIZE(__mem_blocks[i]);
		if (__BLOCK_GET_FREEBLOCK(__mem_blocks[i])==3 && csize>=bsize)
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
		triLogPrint("Not enough memory to allocate %i bytes (largest: %i)!\n",size,triVLargestblock());
		#endif
		return(0);
	}
	
	i = bestblock;
	j = bestblock_prev;	
	triS32 csize = bestblock_size;
	__mem_blocks[i] = __BLOCK_MAKE(bsize,j,0,1);
	
	triS32 next = i+bsize;
	if (csize>bsize && next<__MEM_BLOCKS)
	{
		__mem_blocks[next] = __BLOCK_MAKE(csize-bsize,i,1,1);
		triS32 nextnext = i+csize;
		if (nextnext<__MEM_BLOCKS)
		{
			__BLOCK_SET_PREV(__mem_blocks[nextnext], next);
		}
	}

	__mem_free -= bsize;
	if (__largest_block==csize)		// if we just allocated from one of the largest blocks
	{
		if ((csize-bsize)>(__mem_free/2))
			__largest_block = (csize-bsize);		// there can't be another largest block
		else
			__largest_update = 1;
	}
	
	#ifdef _DEBUG
	triLogPrint("allocated at address %p\n", (triVoid*)(__MEM_START + (i*__BLOCK_SIZE)));
	#endif
	return ((triVoid*)(__MEM_START + (i*__BLOCK_SIZE)));
}


triVoid triVFree( triVoid* ptr )
{
	if (ptr==0) return;

	triS32 block = ((triS32)ptr - __MEM_START)/__BLOCK_SIZE;
	if (block<0 || block>__MEM_BLOCKS)
	{
		#ifdef _DEBUG
		triLogPrint("Block is out of range: %i (%p)\n", block, ptr);
		#endif
		return;
	}
	triS32 csize = __BLOCK_GET_SIZE(__mem_blocks[block]);
	#ifdef _DEBUG
	triLogPrint("freeing block %i (%p), size: %i\n", block, ptr, csize);
	#endif

	if (__BLOCK_GET_FREEBLOCK(__mem_blocks[block])!=1 || csize==0)
	{
		#ifdef _DEBUG
		triLogPrint("Block was not allocated!\n");
		#endif
		return;
	}
	
	// Mark block as free
	__BLOCK_SET_FREE(__mem_blocks[block],1);
	__mem_free += csize;
	
	triS32 next = block+csize;
	// Merge with previous block if possible
	triS32 prev = __BLOCK_GET_PREV(__mem_blocks[block]);
	if (prev<block)
	{
		if (__BLOCK_GET_FREEBLOCK(__mem_blocks[prev])==3)
		{
			__BLOCK_ADD_SIZE(__mem_blocks[prev], csize);
			__BLOCK_SET_BLOCK(__mem_blocks[block],0);	// mark current block as inter block
			if (next<__MEM_BLOCKS)
				__BLOCK_SET_PREV(__mem_blocks[next], prev);
			block = prev;
		}
	}

	// Merge with next block if possible
	if (next<__MEM_BLOCKS)
	{
		if (__BLOCK_GET_FREEBLOCK(__mem_blocks[next])==3)
		{
			__BLOCK_ADD_SIZE(__mem_blocks[block], __BLOCK_GET_SIZE(__mem_blocks[next]));
			__BLOCK_SET_BLOCK(__mem_blocks[next],0);	// mark next block as inter block
			triS32 nextnext = next + __BLOCK_GET_SIZE(__mem_blocks[next]);
			if (nextnext<__MEM_BLOCKS)
				__BLOCK_SET_PREV(__mem_blocks[nextnext], block);
		}
	}

	// Update if a new largest block emerged
	if (__largest_block<__BLOCK_GET_SIZE(__mem_blocks[block])) 
	{
		__largest_block = __BLOCK_GET_SIZE(__mem_blocks[block]);
		__largest_update = 0;		// No update necessary any more, because update only necessary when largest has shrinked at most
	}
}


triU32 triVMemavail()
{
	return __mem_free * __BLOCK_SIZE;
}


triU32 triVLargestblock()
{
	if (__largest_update) __find_largest_block();
	return __largest_block * __BLOCK_SIZE;
}

