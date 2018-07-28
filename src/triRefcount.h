/*
 * triRefcount.h: Header for refcounting ressources
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


#ifndef __TRIREFCOUNT_H__
#define __TRIREFCOUNT_H__


#include "triTypes.h"


/** @defgroup triRefcount Refcount
 *  @{
 */

/** Create a new refcount.
  * Call this after triRefcountRetain returned 0.
  * @param id - string identifying the refcount
  * @param data - pointer to assign to the id
  */
triVoid triRefcountCreate( const triChar* id, triVoid* data );


/** Retain a refcount.
  * Call this to check if the data assigned to id is already in list.
  * @param id - string identifying the refcount
  * @returns Pointer to data assigned to refcount or 0 if id not in list
  */
triVoid* triRefcountRetain( const triChar* id );

/** Retain a refcounted pointer.
  * Call this to check if the data is already in list.
  * @param ptr - Pointer to data to check
  * @returns Pointer to data assigned to refcount or 0 if ptr not in list
  */
triVoid* triRefcountRetainPtr( const triVoid* ptr );

/** Release a refcount.
  * Call this to release a refcount matching the data.
  * @param data - Data to release the refcount from
  * @returns 0 if the data is not referred to any more and can be freed safely
  */
triS32 triRefcountRelease( const triVoid* data );


/** @} */

#endif
