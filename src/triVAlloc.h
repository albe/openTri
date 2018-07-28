/*
 * triVAlloc.h: Header for VRAM allocation routines
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


#ifndef __TRIVALLOC_H__
#define __TRIVALLOC_H__

#include "triTypes.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup triVAlloc VRAM MMU
 *  @{
 */


/** Make a pointer relative to VRAM base.
  * @note A relative NULL pointer is NOT illegal!
  * @param ptr - Pointer to make relative
  * @returns relative pointer matching ptr
  */
triVoid* vrelptr( triVoid *ptr );		// make a pointer relative to memory base address (ATTENTION: A NULL rel ptr is not illegal/invalid!)

/** Make a pointer absolute (useable by CPU).
  * @note A relative NULL pointer is NOT illegal!
  * @param ptr - Pointer to make absolute
  * @returns absolute pointer matching ptr
  */
triVoid* vabsptr( triVoid *ptr );		// make a pointer absolute (default return type of valloc)

/** Allocate memory from VRAM.
  * @param size - Number of bytes to allocate
  * @returns absolute pointer on success, NULL on failure
  */
triVoid* triVAlloc( triU32 size );

/** Free allocated memory from VRAM.
  * @param ptr - Previously allocated pointer
  */
triVoid triVFree( triVoid* ptr );


/** Return remaining unallocated VRAM.
  * @returns Size of remaining VRAM in bytes
  */
triU32 triVMemavail();

/** Return largest free memory block in VRAM.
  * @returns Size of largest free block in bytes
  */
triU32 triVLargestblock();


#ifdef _DEBUG
// Debug printf (to stdout) a trace of the current Memblocks
triVoid __memwalk();
#endif

/** @} */
 

#ifdef __cplusplus
}
#endif

#endif  // __TRIVALLOC_H__
