/*
 * triError.h: Header for generic error message
 * This file is part of the "tri Engine".
 *
 * Copyright (C) 2007 tri
 * Copyright (C) 2007 InsertWittyName <tias_dp@hotmail.com>
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

#ifndef __TRIERROR_H__
#define __TRIERROR_H__

#include "triTypes.h"

/** @defgroup triError Error handling
 *  @{
 */

/**
 * Halt on error
 *
 * @param errmsg - The error message to display
 */
triVoid triError(const triChar *errmsg);

/** @} */

#endif // __TRIERROR_H__
