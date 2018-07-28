#ifndef __TRIHEAP_H__
#define __TRIHEAP_H__

#include "triTypes.h"

typedef struct triHeapStruct triHeap;

struct triHeapStruct
{
	void*			_base;
	triU32			_size;
	triU32			_block_size;
		
	triU32			_free;
	triU32			_largest_block;
	triU32			_largest_update;
	
	triU32			_n_blocks;
	triU32*			_blocks;
};


// Macro to define a heap within a C file without having to call triHeapCreate
#define TRI_HEAP( name, sz, align ) \
	triU8 name##_heap[sz]; \
	triU32 name##_blocks[sz/align] = { (sz/align)|(0x3<<30) }; \
	triHeap name = { ._base = name##_heap, ._size = sz, ._block_size = align, ._free = sz / align, \
	._largest_block = sz / align, ._largest_update = 0, ._n_blocks = sz / align, ._blocks = name##_blocks };


triHeap* triHeapCreate( void* base, triU32 size, triU32 block_size );
void triHeapDestroy( triHeap* mem );

#ifdef _DEBUG
void __triHeapwalk( triHeap* mem );
#endif


void* triHeapAlloc( triHeap* mem, triU32 size );
void* triHeapRealloc( triHeap* mem, void* ptr, triU32 size );
void* triHeapCalloc( triHeap* mem, triU32 size );
void triHeapFree( triHeap* mem, void* ptr );
void triHeapFreeAll( triHeap* mem );

triU32 triHeapMemavail( triHeap* mem );
triU32 triHeapLargestblock( triHeap* mem );
#endif
