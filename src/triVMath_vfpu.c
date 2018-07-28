/*
 * triVMath_vfpu.c: Code for Vector maths using PSP's VFPU
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
 

#include "triVMath_vfpu.h"


#define __asm__ asm volatile


// NOTE: This library depends on triVec4f and triMatrix4f being aligned on 16 bytes!


triVec2f* triVec2Set( triVec2f* a, const triFloat x, const triFloat y )
{
	a->x = x;
	a->y = y;
	return a;
}

triVec3f* triVec3Set( triVec3f* a, const triFloat x, const triFloat y, const triFloat z )
{
	a->x = x;
	a->y = y;
	a->z = z;
	return a;
}

triVec4f* triVec4Set( triVec4f* a, const triFloat x, const triFloat y, const triFloat z, const triFloat w )
{
	a->x = x;
	a->y = y;
	a->z = z;
	a->w = w;
	return a;
}

triVec4f* triVec4Set3( triVec4f* a, const triFloat x, const triFloat y, const triFloat z )
{
	a->x = x;
	a->y = y;
	a->z = z;
	a->w = 1;
	return a;
}

triColor4f* triColor4Set( triColor4f* c, const triFloat r, const triFloat g, const triFloat b, const triFloat a )
{
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = a;
	return c;
}

triColor4f* triColor4Set3( triColor4f* c, const triFloat r, const triFloat g, const triFloat b )
{
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = 1;
	return c;
}

triColor4f* triColor4From4i( triColor4f* a, triColor4i* b )
{
	triFloat f = 1.0f / 255.0f;
	a->r = b->r*f;
	a->g = b->g*f;
	a->b = b->b*f;
	a->a = b->a*f;
	return a;
}

triColor4f* triColor4From8888( triColor4f* a, triColor8888* b )
{
	triFloat f = 1.0f / 255.0f;
	a->r = b->r*f;
	a->g = b->g*f;
	a->b = b->b*f;
	a->a = b->a*f;
	return a;
}

triVec2i* triVec2Ceil( triVec2i* a, const triVec2f* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"vf2id.p		c000, c000, 0\n"		// c000 = ceil(c000 * 1)
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triVec2i* triVec2Trunc( triVec2i* a, const triVec2f* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"vf2iz.p		c000, c000, 0\n"		// c000 = trunc(c000 * 1)
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triVec2i* triVec2Round( triVec2i* a, const triVec2f* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"vf2in.p		c000, c000, 0\n"		// c000 = round(c000 * 1)
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triVec2i* triVec2Floor( triVec2i* a, const triVec2f* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"vf2iu.p		c000, c000, 0\n"		// c000 = floor(c000 * 1)
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}


triVec3i* triVec3Ceil( triVec3i* a, const triVec3f* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"lv.s			s002, 8 + %1\n"			// s002 = b->z
		"vf2id.t		c000, c000, 0\n"		// c000 = ceil(c000 * 1)
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triVec3i* triVec3Trunc( triVec3i* a, const triVec3f* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"lv.s			s002, 8 + %1\n"			// s002 = b->z
		"vf2iz.t		c000, c000, 0\n"		// c000 = trunc(c000 * 1)
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triVec3i* triVec3Round( triVec3i* a, const triVec3f* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"lv.s			s002, 8 + %1\n"			// s002 = b->z
		"vf2in.t		c000, c000, 0\n"		// c000 = round(c000 * 1)
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triVec3i* triVec3Floor( triVec3i* a, const triVec3f* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"lv.s			s002, 8 + %1\n"			// s002 = b->z
		"vf2iu.t		c000, c000, 0\n"		// c000 = floor(c000 * 1)
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}


triVec4i* triVec4Ceil( triVec4i* a, const triVec4f* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *b
		"vf2id.q		c000, c000, 0\n"		// c000 = ceil(c000 * 1)
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triVec4i* triVec4Trunc( triVec4i* a, const triVec4f* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *b
		"vf2iz.q		c000, c000, 0\n"		// c000 = trunc(c000 * 1)
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triVec4i* triVec4Round( triVec4i* a, const triVec4f* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *b
		"vf2in.q		c000, c000, 0\n"		// c000 = round(c000 * 1)
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triVec4i* triVec4Floor( triVec4i* a, const triVec4f* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *b
		"vf2iu.q		c000, c000, 0\n"		// c000 = floor(c000 * 1)
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}



triVec2f* triVec2i2f( triVec2f* a, const triVec2i* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s000 = b->x
		"lv.s			s011, 4 + %1\n"			// s001 = b->y
		"vi2f.p			c000, c010, 0\n"		// c000 = c010
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return(a);
}

triVec3f* triVec3i2f( triVec3f* a, const triVec3i* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s000 = b->x
		"lv.s			s011, 4 + %1\n"			// s001 = b->y
		"lv.s			s012, 8 + %1\n"			// s002 = b->z
		"vi2f.t			c000, c010, 0\n"		// c000 = c010
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return(a);
}

triVec4f* triVec4i2f( triVec4f* a, const triVec4i* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q 			c010, %1\n"				// c010 = *b
		"vi2f.q			c000, c010, 0\n"		// c000 = c010
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return(a);
}


triVec2f* triVec2Rndn( triVec2f* a )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"vrndf2.p		c000\n"					// c000 = rnd(2.0, 4.0)
		"vsub.p			c000, c000, c000[3,3]\n"// c000 = | -1.0 ... 1.0 |
		"vdot.p			s010, c000, c000\n"		// s010 = x*x + y*y
		"vrsq.s			s010, s010\n"			// s010 = 1.0 / s010
		"vscl.p			c000[-1:1,-1:1], c000, s010\n"			// c000 = c000 / s010
		"sv.s			s000, %0\n"
		"sv.s			s001, 4 + %0\n"
		".set			pop\n"					// restore assember option
		: "=m"(*a)
	);
	return(a);
}


triVec3f* triVec3Rndn( triVec3f* a )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"vrndf2.t		c000\n"					// c000 = rnd(2.0, 4.0)
		"vsub.t			c000, c000, c000[3,3,3]\n"	// c000 = | -1.0 ... 1.0 |
		"vdot.t			s010, c000, c000\n"		// s010 = x*x + y*y + z*z
		"vrsq.s			s010, s010\n"			// s010 = 1.0 / s010
		"vscl.t			c000[-1:1,-1:1,-1:1], c000, s010\n"			// c000 = c000 / s010
		"sv.s			s000, 0 + %0\n"
		"sv.s			s001, 4 + %0\n"
		"sv.s			s002, 8 + %0\n"
		".set			pop\n"					// restore assember option
		: "=m"(*a)
	);
	return(a);
}


triVec4f* triVec4Rndn( triVec4f* a )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"vrndf2.q		c000\n"					// c000 = rnd(2.0, 4.0)
		"vsub.q			c000, c000, c000[3,3,3,3]\n"// c000 = | -1.0 ... 1.0 |
		"vdot.q			s010, c000, c000\n"		// s010 = x*x + y*y + z*z + w*w
		"vrsq.s			s010, s010\n"			// s010 = 1.0 / s010
		"vscl.q			c000[-1:1,-1:1,-1:1,-1:1], c000, s010\n"		// c000 = c000 / s010
		"sv.q			c000, %0\n"
		".set			pop\n"					// restore assember option
		: "=m"(*a)
	);
	return(a);
}


triVec4f* triVec4Rndn3( triVec4f* a )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"vrndf2.q		c000\n"					// c000 = rnd(2.0, 4.0)
		"vsub.q			c000, c000, c000[3,3,3,3]\n"// c000 = | -1.0 ... 1.0 |
		"vdot.t			s010, c000, c000\n"		// s010 = x*x + y*y + z*z
		"vrsq.s			s010, s010\n"			// s010 = 1.0 / s010
		"vscl.t			c000[-1:1,-1:1,-1:1], c000, s010\n"		// c000 = c000 / s010
		"sv.q			c000, %0\n"
		".set			pop\n"					// restore assember option
		: "=m"(*a)
	);
	return(a);
}


triVec2f* triVec2Rnd2( triVec2f* a )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"vrndf2.p		c000\n"					// c000 = rnd(2.0, 4.0)
		"vsub.p			c000, c000, c000[3,3]\n"// c000 = | -1.0 ... 1.0 |
		"sv.s			s000, %0\n"
		"sv.s			s001, 4 + %0\n"
		".set			pop\n"					// restore assember option
		: "=m"(*a)
	);
	return(a);
}


triVec3f* triVec3Rnd2( triVec3f* a )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"vrndf2.t		c000\n"					// c000 = rnd(2.0, 4.0)
		"vsub.t			c000, c000, c000[3,3,3]\n"	// c000 = | -1.0 ... 1.0 |
		"sv.s			s000, 0 + %0\n"
		"sv.s			s001, 4 + %0\n"
		"sv.s			s002, 8 + %0\n"
		".set			pop\n"					// restore assember option
		: "=m"(*a)
	);
	return(a);
}


triVec4f* triVec4Rnd2( triVec4f* a )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"vrndf2.q		c000\n"					// c000 = rnd(2.0, 4.0)
		"vsub.q			c000, c000, c000[3,3,3,3]\n"// c000 = | -1.0 ... 1.0 |
		"sv.q			c000, %0\n"
		".set			pop\n"					// restore assember option
		: "=m"(*a)
	);
	return(a);
}


triVec2f* triVec2Rnd( triVec2f* a )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"vone.p			c010\n"
		"vrndf1.p		c000\n"					// c000 = rnd(1.0, 2.0)
		"vsub.p			c000, c000, c010\n"		// c000 = | 0.0 ... 1.0 |
		"sv.s			s000, %0\n"
		"sv.s			s001, 4 + %0\n"
		".set			pop\n"					// restore assember option
		: "=m"(*a)
	);
	return(a);
}


triVec3f* triVec3Rnd( triVec3f* a )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"vone.t			c010\n"
		"vrndf1.t		c000\n"					// c000 = rnd(1.0, 2.0)
		"vsub.t			c000, c000, c010\n"		// c000 = | 0.0 ... 1.0 |
		"sv.s			s000, 0 + %0\n"
		"sv.s			s001, 4 + %0\n"
		"sv.s			s002, 8 + %0\n"
		".set			pop\n"					// restore assember option
		: "=m"(*a)
	);
	return(a);
}


triVec4f* triVec4Rnd( triVec4f* a )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"vone.q			c010\n"
		"vrndf1.q		c000\n"					// c000 = rnd(1.0, 2.0)
		"vsub.q			c000, c000, c010\n"// c000 = | 0.0 ... 1.0 |
		"sv.q			c000, %0\n"
		".set			pop\n"					// restore assember option
		: "=m"(*a)
	);
	return(a);
}


triVec2f* triVec2Add( triVec2f* a, const triVec2f* b, const triVec2f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"vadd.p			c000, c010, c020\n"		// c000 = c010 + c020
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}

triVec3f* triVec3Add( triVec3f* a, const triVec3f* b, const triVec3f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s012, 8 + %1\n"			// s012 = b->z
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"lv.s			s022, 8 + %2\n"			// s022 = c->z
		"vadd.t			c000, c010, c020\n"		// c000 = c010 + c020
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}

triVec4f* triVec4Add( triVec4f* a, const triVec4f* b, const triVec4f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q 			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vadd.q			c000, c010, c020\n"		// c000 = c010 + c020
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}

triVec4f* triVec4Add3( triVec4f* a, const triVec4f* b, const triVec4f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q 			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vadd.t			c010, c010, c020\n"		// c010 = c010 + c020
		"sv.q			c010, %0\n"				// *a = c010
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}

triVec2f* triVec2Sub( triVec2f* a, const triVec2f* b, const triVec2f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"vsub.p			c000, c010, c020\n"		// c000 = c010 - c020
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}

triVec3f* triVec3Sub( triVec3f* a, const triVec3f* b, const triVec3f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s012, 8 + %1\n"			// s012 = b->z
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"lv.s			s022, 8 + %2\n"			// s022 = c->z
		"vsub.t			c000, c010, c020\n"		// c000 = c010 - c020
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}

triVec4f* triVec4Sub( triVec4f* a, const triVec4f* b, const triVec4f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q 			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vsub.q			c000, c010, c020\n"		// c000 = c010 - c020
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}

triVec4f* triVec4Sub3( triVec4f* a, const triVec4f* b, const triVec4f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q 			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vsub.t			c010, c010, c020\n"		// c010 = c010 - c020
		"sv.q			c010, %0\n"				// *a = c010
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}

triVec2f* triVec2Mul( triVec2f* a, const triVec2f* b, const triVec2f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"vmul.p			c000, c010, c020\n"		// c000 = c010 * c020
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}

triVec3f* triVec3Mul( triVec3f* a, const triVec3f* b, const triVec3f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s012, 8 + %1\n"			// s012 = b->z
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"lv.s			s022, 8 + %2\n"			// s022 = c->z
		"vmul.t			c000, c010, c020\n"		// c000 = c010 * c020
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}

triVec4f* triVec4Mul( triVec4f* a, const triVec4f* b, const triVec4f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q 			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vmul.q			c000, c010, c020\n"		// c000 = c010 * c020
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}

triVec4f* triVec4Mul3( triVec4f* a, const triVec4f* b, const triVec4f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q 			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vmul.t			c010, c010, c020\n"		// c000 = c010 * c020
		"sv.q			c010, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}

triVec2f* triVec2Div( triVec2f* a, const triVec2f* b, const triVec2f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"vdiv.p			c000, c010, c020\n"		// c000 = c010 / c020
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}

triVec3f* triVec3Div( triVec3f* a, const triVec3f* b, const triVec3f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s012, 8 + %1\n"			// s012 = b->z
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"lv.s			s022, 8 + %2\n"			// s022 = c->z
		"vdiv.t			c000, c010, c020\n"		// c000 = c010 / c020
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}

triVec4f* triVec4Div( triVec4f* a, const triVec4f* b, const triVec4f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q 			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vdiv.q			c000, c010, c020\n"		// c000 = c010 / c020
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}

triVec4f* triVec4Div3( triVec4f* a, const triVec4f* b, const triVec4f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q 			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vdiv.t			c010, c010, c020\n"		// c000 = c010 / c020
		"sv.q			c010, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}


// FIXME: pointer aliasing problem with strict-aliasing rules on GCC!
triVec2f* triVec2Neg(triVec2f *a, const triVec2f *b)
{
	triVec2i* ai = (triVec2i*)a;
	const triVec2i* bi = (const triVec2i*)b;

	ai->x = bi->x ^ 0x80000000U;
	ai->y = bi->y ^ 0x80000000U;
	
	return (a);
}

triVec3f* triVec3Neg(triVec3f *a, const triVec3f *b)
{
	triVec3i* ai = (triVec3i*)a;
	const triVec3i* bi = (const triVec3i*)b;

	ai->x = bi->x ^ 0x80000000U;
	ai->y = bi->y ^ 0x80000000U;
	ai->z = bi->z ^ 0x80000000U;
	
	return (a);
}

triVec4f* triVec4Neg(triVec4f* a, const triVec4f* b)
{
	triVec4i* ai = (triVec4i*)a;
	const triVec4i* bi = (const triVec4i*)b;

	ai->x = bi->x ^ 0x80000000U;
	ai->y = bi->y ^ 0x80000000U;
	ai->z = bi->z ^ 0x80000000U;
	ai->w = bi->w ^ 0x80000000U;
	
	return (a);
}

triVec4f* triVec4Neg3(triVec4f* a, const triVec4f* b)
{
	triVec4i* ai = (triVec4i*)a;
	const triVec4i* bi = (const triVec4i*)b;

	ai->x = bi->x ^ 0x80000000U;
	ai->y = bi->y ^ 0x80000000U;
	ai->z = bi->z ^ 0x80000000U;
	
	return (a);
}


triVec2f* triVec2Abs(triVec2f *a, const triVec2f *b)
{
	triVec2i* ai = (triVec2i*)a;
	const triVec2i* bi = (const triVec2i*)b;

	ai->x = bi->x & 0x7FFFFFFFU;
	ai->y = bi->y & 0x7FFFFFFFU;

	return (a);
}

triVec3f* triVec3Abs(triVec3f *a, const triVec3f *b)
{
	triVec3i* ai = (triVec3i*)a;
	const triVec3i* bi = (const triVec3i*)b;

	ai->x = bi->x & 0x7FFFFFFFU;
	ai->y = bi->y & 0x7FFFFFFFU;
	ai->z = bi->z & 0x7FFFFFFFU;

	return (a);
}

triVec4f* triVec4Abs(triVec4f* a, const triVec4f* b)
{
	triVec4i* ai = (triVec4i*)a;
	const triVec4i* bi = (const triVec4i*)b;

	ai->x = bi->x & 0x7FFFFFFFU;
	ai->y = bi->y & 0x7FFFFFFFU;
	ai->z = bi->z & 0x7FFFFFFFU;
	ai->w = bi->w & 0x7FFFFFFFU;
	
	return (a);
}

triVec4f* triVec4Abs3(triVec4f* a, const triVec4f* b)
{
	triVec4i* ai = (triVec4i*)a;
	const triVec4i* bi = (const triVec4i*)b;

	ai->x = bi->x & 0x7FFFFFFFU;
	ai->y = bi->y & 0x7FFFFFFFU;
	ai->z = bi->z & 0x7FFFFFFFU;
	
	return (a);
}


triVec2f* triVec2Clamp(triVec2f *a, const triVec2f *b, triFloat min, triFloat max)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"mfc1			$8,   %2\n"				// t0 = min
		"mfc1			$9,   %3\n"				// t1 = max
		"mtv			$8,   s010\n"			// s010 = t0 = min
		"mtv			$9,   s011\n"			// s011 = t1 = max
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"vmax.p			c000, c000, c010[X,X]\n"	// c000 = max(c000, [s010,s010,s010,s010])
		"vmin.p			c000, c000, c010[Y,Y]\n"	// c000 = min(c000, [s011,s011,s011,s011])
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "f"(min), "f"(max)
		: "$8", "$9"
	);
	return (a);
}

triVec2f* triVec2Min(triVec2f* a, const triVec2f* b, const triVec2f* c)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"vmin.p			c000, c010, c020\n"		// c000 = min(c010, c020)
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);

	return (a);
}

triVec2f* triVec2Max(triVec2f* a, const triVec2f* b, const triVec2f* c)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"vmax.p			c000, c010, c020\n"		// c000 = max(c010, c020)
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);

	return (a);
}


triVec3f* triVec3Clamp(triVec3f *a, const triVec3f *b, triFloat min, triFloat max)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"mfc1			$8,   %2\n"				// t0 = min
		"mfc1			$9,   %3\n"				// t1 = max
		"mtv			$8,   s010\n"			// s010 = t0 = min
		"mtv			$9,   s011\n"			// s011 = t1 = max
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"lv.s			s002, 8 + %1\n"			// s002 = b->z
		"vmax.t			c000, c000, c010[X,X,X]\n"	// c000 = max(c000, [s010,s010,s010,s010])
		"vmin.t			c000, c000, c010[Y,Y,Y]\n"	// c000 = min(c000, [s011,s011,s011,s011])
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "f"(min), "f"(max)
		: "$8", "$9"
	);
	return (a);
}

triVec3f* triVec3Min(triVec3f* a, const triVec3f* b, const triVec3f* c)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s012, 8 + %1\n"			// s012 = b->z
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"lv.s			s022, 8 + %2\n"			// s022 = c->z
		"vmin.t			c000, c010, c020\n"		// c000 = min(c010, c020)
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);

	return (a);
}

triVec3f* triVec3Max(triVec3f* a, const triVec3f* b, const triVec3f* c)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s012, 8 + %1\n"			// s012 = b->z
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"lv.s			s022, 8 + %2\n"			// s022 = c->z
		"vmax.t			c000, c010, c020\n"		// c000 = max(c010, c020)
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);

	return (a);
}

triVec4f* triVec4Clamp(triVec4f* a, const triVec4f* b, triFloat min, triFloat max)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"mfc1			$8,   %2\n"				// t0 = min
		"mfc1			$9,   %3\n"				// t1 = max
		"mtv			$8,   s010\n"			// s010 = t0 = min
		"mtv			$9,   s011\n"			// s011 = t1 = max
		"lv.q			c000, %1\n"				// c000 = *b
		"vmax.q			c000, c000, c010[X,X,X,X]\n"	// c000 = max(c000, [s010,s010,s010,s010])
		"vmin.q			c000, c000, c010[Y,Y,Y,Y]\n"	// c000 = min(c000, [s011,s011,s011,s011])
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "f"(min), "f"(max)
		: "$8", "$9"
	);
	return (a);
}

triVec4f* triVec4Clamp3(triVec4f* a, const triVec4f* b, triFloat min, triFloat max)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"mfc1			$8,   %2\n"				// t0 = min
		"mfc1			$9,   %3\n"				// t1 = max
		"mtv			$8,   s010\n"			// s010 = t0 = min
		"mtv			$9,   s011\n"			// s011 = t1 = max
		"lv.q			c000, %1\n"				// c000 = *b
		"vmax.t			c000, c000, c010[X,X,X]\n"	// c000 = max(c000, [s010,s010,s010])
		"vmin.t			c000, c000, c010[Y,Y,Y]\n"	// c000 = min(c000, [s011,s011,s011])
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "f"(min), "f"(max)
		: "$8", "$9"
	);
	return (a);
}

triVec4f* triVec4Min(triVec4f* a, const triVec4f* b, const triVec4f* c)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vmin.q			c000, c010, c020\n"		// c000 = min(c010, c020)
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);

	return (a);
}

triVec4f* triVec4Max(triVec4f* a, const triVec4f* b, const triVec4f* c)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vmax.q			c000, c010, c020\n"		// c000 = max(c010, c020)
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);

	return (a);
}


triFloat triVec2Sum(const triVec2f* a)
{
	triFloat v;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = a->x
		"lv.s			s001, 4 + %1\n"			// s001 = a->y
		"vfad.p			s000, c000\n"			// s000 = funneladd(c000)
		"sv.s			s000, %0\n"				// v    = s000
		".set			pop\n"					// restore assember option
		: "=m"(v)
		: "m"(*a)
	);
	return (v);
}

triFloat triVec2Avg(const triVec2f* a)
{
	triFloat v;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = a->x
		"lv.s			s001, 4 + %1\n"			// s001 = a->y
		"vavg.p			s000, c000\n"			// s000 = funnelavg(c000)
		"sv.s			s000, %0\n"				// v    = s000
		".set			pop\n"					// restore assember option
		: "=m"(v)
		: "m"(*a)
	);
	return (v);
}

triFloat triVec3Sum(const triVec3f* a)
{
	triFloat v;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = a->x
		"lv.s			s001, 4 + %1\n"			// s001 = a->y
		"lv.s			s002, 8 + %1\n"			// s002 = a->z
		"vfad.t			s000, c000\n"			// s000 = funneladd(c000)
		"sv.s			s000, %0\n"				// v    = s000
		".set			pop\n"					// restore assember option
		: "=m"(v)
		: "m"(*a)
	);
	return (v);
}

triFloat triVec3Avg(const triVec3f* a)
{
	triFloat v;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = a->x
		"lv.s			s001, 4 + %1\n"			// s001 = a->y
		"lv.s			s002, 8 + %1\n"			// s002 = a->z
		"vavg.t			s000, c000\n"			// s000 = funnelavg(c000)
		"sv.s			s000, %0\n"				// v    = s000
		".set			pop\n"					// restore assember option
		: "=m"(v)
		: "m"(*a)
	);
	return (v);
}

triFloat triVec4Sum(const triVec4f* a)
{
	triFloat v;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *a
		"vfad.q			s000, c000\n"			// s000 = funneladd(c000)
		"sv.s			s000, %0\n"				// v    = s000
		".set			pop\n"					// restore assember option
		: "=m"(v)
		: "m"(*a)
	);
	return (v);
}

triFloat triVec4Sum3(const triVec4f* a)
{
	triFloat v;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *a
		"vfad.t			s000, c000\n"			// s000 = funneladd(c000)
		"sv.s			s000, %0\n"				// v    = s000
		".set			pop\n"					// restore assember option
		: "=m"(v)
		: "m"(*a)
	);
	return (v);
}

triFloat triVec4Avg(const triVec4f* a)
{
	triFloat v;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *a
		"vavg.q			s000, c000\n"			// s000 = funnelavg(c000)
		"sv.s			s000, %0\n"				// v    = s000
		".set			pop\n"					// restore assember option
		: "=m"(v)
		: "m"(*a)
	);
	return (v);
}

triFloat triVec4Avg3(const triVec4f* a)
{
	triFloat v;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *a
		"vavg.t			s000, c000\n"			// s000 = funnelavg(c000)
		"sv.s			s000, %0\n"				// v    = s000
		".set			pop\n"					// restore assember option
		: "=m"(v)
		: "m"(*a)
	);
	return (v);
}


triVec2f* triVec2Sgn(triVec2f* a, const triVec2f* b)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"vsgn.p			c000, c000\n"			// c000 = sign(c000)
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triVec2f* triVec2Normalize(triVec2f* a, const triVec2f* b)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"vdot.p			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002 + s003*s003
		"vzero.s		s011\n"					// s011 = 0
		"vcmp.s			EZ, s010\n"				// CC[0] = (s010==0.0f)
		"vrsq.s			s010, s010\n"			// s010 = 1.0 / sqrt(s010)
		"vcmovt.s		s010, s011, 0\n"		// if (CC[0]) s010 = s011
		"vscl.p			c000[-1:1,-1:1], c000, s010\n"	// c000 = c000 * s010
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triFloat triVec2Length(const triVec2f* a)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = a->x
		"lv.s			s001, 4 + %1\n"			// s001 = a->y
		"vdot.t			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002
		"vsqrt.s		s010, s010\n"			// s010 = sqrt(s010)
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a)
	);
	return (f);
}

triFloat triVec2SquareLength(const triVec2f* a)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = a->x
		"lv.s			s001, 4 + %1\n"			// s001 = a->y
		"vdot.t			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a)
	);
	return (f);
}

triFloat triVec2Dist(const triVec2f* a, const triVec2f* b)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = a->x
		"lv.s			s001, 4 + %1\n"			// s001 = a->y
		"lv.s			s010, 0 + %2\n"			// s000 = b->x
		"lv.s			s011, 4 + %2\n"			// s001 = b->y
		"vsub.t			c000, c000, c010\n"		// c000 = c000 - c010
		"vdot.t			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002
		"vsqrt.s		s010, s010\n"			// s010 = sqrt(s010)
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a), "m"(*b)
	);
	return (f);
}

triFloat triVec2SquareDist(const triVec2f* a, const triVec2f* b)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = a->x
		"lv.s			s001, 4 + %1\n"			// s001 = a->y
		"lv.s			s010, 0 + %2\n"			// s000 = b->x
		"lv.s			s011, 4 + %2\n"			// s001 = b->y
		"vsub.t			c000, c000, c010\n"		// c000 = c000 - c010
		"vdot.t			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a), "m"(*b)
	);
	return (f);
}

triVec2f* triVec2Lerp(triVec2f* a, const triVec2f* b, const triVec2f* c, triFloat t)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"mfc1			$8,   %3\n"				// t0   = t
		"mtv			$8,   s030\n"			// s030 = t0
		"lv.s			s010, 0 + %1\n"			// s000 = b->x
		"lv.s			s011, 4 + %1\n"			// s001 = b->y
		"lv.s			s020, 0 + %2\n"			// s000 = c->x
		"lv.s			s021, 4 + %2\n"			// s001 = c->y
		"vsub.p			c000, c020, c010\n"		// c000 = c020 - c010 = (v2 - v1)
		"vscl.p			c000, c000, s030\n"		// c000 = c000 * s030 = (v2 - v1) * t
		"vadd.p			c010, c010, c000\n"		// c010 = c010 + c000 = v1 + t * (v2 - v1)
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c), "f"(t)
		: "$8"
	);
	return (a);
}

triVec2f* triVec2Scale(triVec2f* a, const triVec2f* b, triFloat t)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"mfc1			$8,   %2\n"				// t0   = t
		"mtv			$8,   s010\n"			// s010 = t0
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"vscl.p			c000, c000, s010\n"		// c000 = c000 * s010
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "f"(t)
		: "$8"
	);

	return (a);
}



triVec3f* triVec3Sgn(triVec3f* a, const triVec3f* b)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"lv.s			s002, 8 + %1\n"			// s002 = b->z
		"vsgn.t			c000, c000\n"			// c000 = sign(c000)
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triVec3f* triVec3Normalize(triVec3f* a, const triVec3f* b)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"lv.s			s002, 8 + %1\n"			// s002 = b->z
		"vdot.t			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002 + s003*s003
		"vzero.s		s011\n"					// s011 = 0
		"vcmp.s			EZ, s010\n"				// CC[0] = (s010==0.0f)
		"vrsq.s			s010, s010\n"			// s010 = 1.0 / sqrt(s010)
		"vcmovt.s		s010, s011, 0\n"		// if (CC[0]) s010 = s011
		"vscl.t			c000[-1:1,-1:1,-1:1], c000, s010\n"	// c000 = c000 * s010
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triFloat triVec3Length(const triVec3f* a)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = a->x
		"lv.s			s001, 4 + %1\n"			// s001 = a->y
		"lv.s			s002, 8 + %1\n"			// s002 = a->z
		"vdot.t			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002
		"vsqrt.s		s010, s010\n"			// s010 = sqrt(s010)
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a)
	);
	return (f);
}

triFloat triVec3SquareLength(const triVec3f* a)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = a->x
		"lv.s			s001, 4 + %1\n"			// s001 = a->y
		"lv.s			s002, 8 + %1\n"			// s002 = a->z
		"vdot.t			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a)
	);
	return (f);
}

triFloat triVec3Dist(const triVec3f* a, const triVec3f* b)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = a->x
		"lv.s			s001, 4 + %1\n"			// s001 = a->y
		"lv.s			s002, 8 + %1\n"			// s002 = a->z
		"lv.s			s010, 0 + %2\n"			// s010 = b->x
		"lv.s			s011, 4 + %2\n"			// s011 = b->y
		"lv.s			s012, 8 + %2\n"			// s012 = b->z
		"vsub.t			c000, c000, c010\n"		// c000 = c000 - c010
		"vdot.t			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002
		"vsqrt.s		s010, s010\n"			// s010 = sqrt(s010)
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a), "m"(*b)
	);
	return (f);
}

triFloat triVec3SquareDist(const triVec3f* a, const triVec3f* b)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s000, 0 + %1\n"			// s000 = a->x
		"lv.s			s001, 4 + %1\n"			// s001 = a->y
		"lv.s			s002, 8 + %1\n"			// s002 = a->z
		"lv.s			s010, 0 + %2\n"			// s010 = b->x
		"lv.s			s011, 4 + %2\n"			// s011 = b->y
		"lv.s			s012, 8 + %2\n"			// s012 = b->z
		"vsub.t			c000, c000, c010\n"		// c000 = c000 - c010
		"vdot.t			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a), "m"(*b)
	);
	return (f);
}

triVec3f* triVec3Lerp(triVec3f* a, const triVec3f* b, const triVec3f* c, triFloat t)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"mfc1			$8,   %3\n"				// t0   = t
		"mtv			$8,   s030\n"			// s030 = t0
		"lv.s			s010, 0 + %1\n"			// s000 = b->x
		"lv.s			s011, 4 + %1\n"			// s001 = b->y
		"lv.s			s012, 8 + %1\n"			// s002 = b->z
		"lv.s			s020, 0 + %2\n"			// s010 = c->x
		"lv.s			s021, 4 + %2\n"			// s011 = c->y
		"lv.s			s022, 8 + %2\n"			// s012 = c->z
		"vsub.t			c000, c020, c010\n"		// c000 = c020 - c010 = (v2 - v1)
		"vscl.t			c000, c000, s030\n"		// c000 = c000 * s030 = (v2 - v1) * t
		"vadd.t			c010, c010, c000\n"		// c010 = c010 + c000 = v1 + t * (v2 - v1)
		"sv.s			s010, 0 + %0\n"			// a->x = s000
		"sv.s			s011, 4 + %0\n"			// a->y = s001
		"sv.s			s012, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c), "f"(t)
		: "$8"
	);
	return (a);
}

triVec3f* triVec3Scale(triVec3f* a, const triVec3f* b, triFloat t)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"mfc1			$8,   %2\n"				// t0   = t
		"mtv			$8,   s010\n"			// s010 = t0
		"lv.s			s000, 0 + %1\n"			// s000 = b->x
		"lv.s			s001, 4 + %1\n"			// s001 = b->y
		"lv.s			s002, 8 + %1\n"			// s002 = b->z
		"vscl.t			c000, c000, s010\n"		// c000 = c000 * s010
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "f"(t)
		: "$8"
	);

	return (a);
}



triVec4f* triVec4Sgn(triVec4f* a, const triVec4f* b)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *b
		"vsgn.q			c000, c000\n"			// c000 = sign(c000)
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triVec4f* triVec4Normalize(triVec4f* a, const triVec4f* b)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *b
		"vdot.q			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002 + s003*s003
		"vzero.s		s011\n"					// s011 = 0
		"vcmp.s			EZ, s010\n"				// CC[0] = (s010==0.0f)
		"vrsq.s			s010, s010\n"			// s010 = 1.0 / sqrt(s010)
		"vcmovt.s		s010, s011, 0\n"		// if (CC[0]) s010 = s011
		"vscl.q			c000[-1:1,-1:1,-1:1,-1:1], c000, s010\n"	// c000 = c000 * s010
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triVec4f* triVec4Normalize3(triVec4f* a, const triVec4f* b)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *b
		"vdot.t			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002
		"vzero.s		s011\n"					// s011 = 0
		"vcmp.s			EZ, s010\n"				// CC[0] = (s010==0.0f)
		"vrsq.s			s010, s010\n"			// s010 = 1.0 / sqrt(s010)
		"vcmovt.s		s010, s011, 0\n"		// if (CC[0]) s010 = s011
		"vscl.t			c000[-1:1,-1:1,-1:1], c000, s010\n"	// c000 = c000 * s010
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}

triFloat triVec4Length(const triVec4f* a)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *a
		"vdot.q			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002 + s003*s003
		"vsqrt.s		s010, s010\n"			// s010 = sqrt(s010)
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a)
	);
	return (f);
}

triFloat triVec4Length3(const triVec4f* a)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *a
		"vdot.t			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002
		"vsqrt.s		s010, s010\n"			// s010 = sqrt(s010)
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a)
	);
	return (f);
}

triFloat triVec4SquareLength(const triVec4f* a)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *a
		"vdot.q			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002 + s003*s003
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a)
	);
	return (f);
}

triFloat triVec4SquareLength3(const triVec4f* a)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *a
		"vdot.t			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a)
	);
	return (f);
}

triFloat triVec4Dist(const triVec4f* a, const triVec4f* b)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *a
		"lv.q			c010, %2\n"				// c010 = *b
		"vsub.q			c000, c000, c010\n"		// c000 = c000 - c010
		"vdot.q			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002 + s003*s003
		"vsqrt.s		s010, s010\n"			// s010 = sqrt(s010)
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a), "m"(*b)
	);
	return (f);
}

triFloat triVec4Dist3(const triVec4f* a, const triVec4f* b)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *a
		"lv.q			c010, %2\n"				// c010 = *b
		"vsub.t			c000, c000, c010\n"		// c000 = c000 - c010
		"vdot.t			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002
		"vsqrt.s		s010, s010\n"			// s010 = sqrt(s010)
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a), "m"(*b)
	);
	return (f);
}

triFloat triVec4SquareDist(const triVec4f* a, const triVec4f* b)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *a
		"lv.q			c010, %2\n"				// c010 = *b
		"vsub.q			c000, c000, c010\n"		// c000 = c000 - c010
		"vdot.q			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002 + s003*s003
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a), "m"(*b)
	);
	return (f);
}

triFloat triVec4SquareDist3(const triVec4f* a, const triVec4f* b)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *a
		"lv.q			c010, %2\n"				// c010 = *b
		"vsub.t			c000, c000, c010\n"		// c000 = c000 - c010
		"vdot.t			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002
		"sv.s			s010, %0\n"				// f    = s010
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a), "m"(*b)
	);
	return (f);
}

triVec4f* triVec4Lerp(triVec4f* a, const triVec4f* b, const triVec4f* c, triFloat t)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"mfc1			$8,   %3\n"				// t0   = t
		"mtv			$8,   s030\n"			// s030 = t0
		"lv.q			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vsub.q			c000, c020, c010\n"		// c000 = c020 - c010 = (v2 - v1)
		"vscl.q			c000, c000, s030\n"		// c000 = c000 * s030 = (v2 - v1) * t
		"vadd.q			c010, c010, c000\n"		// c010 = c010 + c000 = v1 + t * (v2 - v1)
		"sv.q			c010, %0\n"				// *a = c010
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c), "f"(t)
		: "$8"
	);
	return (a);
}

triVec4f* triVec4Lerp3(triVec4f* a, const triVec4f* b, const triVec4f* c, triFloat t)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"mfc1			$8,   %3\n"				// t0   = t
		"mtv			$8,   s030\n"			// s030 = t0
		"lv.q			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vsub.t			c000, c020, c010\n"		// c000 = c020 - c010 = (v2 - v1)
		"vscl.t			c000, c000, s030\n"		// c000 = c000 * s030 = (v2 - v1) * t
		"vadd.t			c010, c010, c000\n"		// c010 = c010 + c000 = v1 + t * (v2 - v1)
		"sv.q			c010, %0\n"				// *a = c010
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c), "f"(t)
		: "$8"
	);
	return (a);
}

triVec4f* triVec4Scale(triVec4f* a, const triVec4f* b, triFloat t)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"mfc1			$8,   %2\n"				// t0   = t
		"mtv			$8,   s010\n"			// s010 = t0
		"lv.q			c000, %1\n"				// c000 = *b
		"vscl.q			c000, c000, s010\n"		// c000 = c000 * s010
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "f"(t)
		: "$8"
	);

	return (a);
}

triVec4f* triVec4Scale3(triVec4f* a, const triVec4f* b, triFloat t)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"mfc1			$8,   %2\n"				// t0   = t
		"mtv			$8,   s010\n"			// s010 = t0
		"lv.q			c000, %1\n"				// c000 = *b
		"vscl.t			c000, c000, s010\n"		// c000 = c000 * s010
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "f"(t)
		: "$8"
	);

	return (a);
}



triFloat triVec2Dot(const triVec2f* a, const triVec2f* b)
{
	triFloat v;

	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"vdot.p			s000, c010, c020\n"		// s000 = c010 dot c020
		"sv.s			s000, %0\n"				// v    = s000
		".set			pop\n"					// restore assember option
		: "=m"(v)
		: "m"(*a), "m"(*b)
	);
	
	return v;
}

triFloat triVec3Dot(const triVec3f* a, const triVec3f* b)
{
	triFloat v;

	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s012, 8 + %1\n"			// s012 = b->z
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"lv.s			s022, 8 + %2\n"			// s022 = c->z
		"vdot.t			s000, c010, c020\n"		// s000 = c010 dot c020
		"sv.s			s000, %0\n"				// v    = s000
		".set			pop\n"					// restore assember option
		: "=m"(v)
		: "m"(*a), "m"(*b)
	);
	
	return v;
}

triFloat triVec4Dot(const triVec4f* a, const triVec4f* b)
{
	triFloat v;

	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c010, %1\n"				// c010 = *a
		"lv.q			c020, %2\n"				// c020 = *b
		"vdot.q			s000, c010, c020\n"		// s000 = c010 dot c020
		"sv.s			s000, %0\n"				// v    = s000
		".set			pop\n"					// restore assember option
		: "=m"(v)
		: "m"(*a), "m"(*b)
	);
	
	return v;
}

triFloat triVec4Dot3(const triVec4f* a, const triVec4f* b)
{
	triFloat v;

	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c010, %1\n"				// c010 = *a
		"lv.q			c020, %2\n"				// c020 = *b
		"vdot.t			s000, c010, c020\n"		// s000 = c010 dot c020
		"sv.s			s000, %0\n"				// v    = s000
		".set			pop\n"					// restore assember option
		: "=m"(v)
		: "m"(*a), "m"(*b)
	);
	
	return v;
}


triVec3f* triVec3Cross(triVec3f* a, const triVec3f* b, const triVec3f* c)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s012, 8 + %1\n"			// s012 = b->z
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"lv.s			s022, 8 + %2\n"			// s022 = c->z
		"vcrsp.t		c000, c010, c020\n"		// c000 = c010 x c020
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return (a);
}


triVec4f* triVec4Cross(triVec4f* a, const triVec4f* b, const triVec4f* c)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vcrsp.t		c010, c010, c020\n"		// c010 = c010 x c020
		"sv.q			c010, %0\n"				// *a = s010
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return (a);
}



triVec2f* triVec2Reflect(triVec2f* a, const triVec2f* b, const triVec2f* c)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"vdot.p			s031, c010, c020\n"		// s031 = s010*s020 + s011*s021 + s012*s022
		"vfim.s			s030, -2.0\n"			// s030 = -2.0f
		"vmul.s			s032, s030, s031\n"		// s032 = s030 * s031
		"vscl.p			c020, c020, s032\n"		// c020 = c020 * s032
		"vadd.p			c000, c010, c020\n"		// c000 = c010 + c020
		"vdot.p			s033, c000, c000\n"		// s033 = c000 * c000
		"vcmp.s			EZ, s033\n"				// CC[0] = (s033==0.0f)
		"vrsq.s			s033, s033\n"			// s033 = 1.0 / sqrt(s033)
		"vcmovt.s		s033, s033[0], 0\n"		// if (CC[0]) s033 = 0
		"vscl.p			c000[-1:1,-1:1], c000, s033\n"
												// c000 = c000 * s033
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return (a);
}

triVec2f* triVec2Refract(triVec2f* a, const triVec2f* b, const triVec2f* c, const triFloat eta)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"lv.s			s030, %3\n"				// s030 = eta
		"vdot.p			s031, c010, c020\n"		// s031 = s010*s020 + s011*s021 + s012*s022
		"vscl.p			c010, c010, s030\n"		// c010 = c010 * s030 = *b * eta
		"vmul.s			s032, s030, s030\n"		// s032 = s030 * s030 = eta * eta
		"vmul.s			s033, s031, s031\n"		// s033 = s031 * s031 = d * d
		"vmul.s			s031, s031, s030\n"		// s031 = s031 * s030 = d * eta
		"vocp.s			s033, s033\n"			// s033 = 1.0 - s033  = 1 - d*d
		"vmul.s			s033, s032, s033\n"		// s033 = s033 * s032 = eta * eta * (1 - d*d)
		"vocp.s			s033, s033\n"			// s033 = 1.0 - s033  = 1 - eta * eta * (1 - d*d)
		"vsqrt.s		s033, s033\n"			// s033 = sqrt(s033)  = sqrt(1 - eta * eta * (1 - d*d))
		"vsub.s			s031, s031, s033\n"		// s031 = s031 - s032 = d * eta - sqrt(1 - eta * eta * (1 - d*d))
		"vscl.p			c020, c020, s031\n"		// c020 = c020 * s031
		"vadd.p			c000, c010, c020\n"		// c000 = c010 + c020
		"vdot.p			s033, c000, c000\n"		// s033 = c000 * c000
		"vcmp.p			ES, c000\n"				// CC[4] = isinfornan(s000) | isinfornan(s001) | isinfornan(s002)
		"vrsq.s			s033, s033\n"			// s033 = 1.0 / sqrt(s033)
		"vscl.p			c000[-1:1,-1:1], c000, s033\n"
												// c000 = c000 * s033
		"vcmovt.p		c000, c000[0,0], 4\n"	// if (CC[4]) c000 = 0
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c), "m"(eta)
	);
	return (a);
}


triVec3f* triVec3Reflect(triVec3f* a, const triVec3f* b, const triVec3f* c)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s012, 8 + %1\n"			// s012 = b->z
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"lv.s			s022, 8 + %2\n"			// s022 = c->z
		"vdot.t			s031, c010, c020\n"		// s031 = s010*s020 + s011*s021 + s012*s022
		"vfim.s			s030, -2.0\n"			// s030 = -2.0f
		"vmul.s			s032, s030, s031\n"		// s032 = s030 * s031
		"vscl.t			c020, c020, s032\n"		// c020 = c020 * s032
		"vadd.t			c000, c010, c020\n"		// c000 = c010 + c020
		"vdot.t			s033, c000, c000\n"		// s033 = c000 * c000
		"vcmp.s			EZ, s033\n"				// CC[0] = (s033==0.0f)
		"vrsq.s			s033, s033\n"			// s033 = 1.0 / sqrt(s033)
		"vcmovt.s		s033, s033[0], 0\n"		// if (CC[0]) s033 = 0
		"vscl.t			c000[-1:1,-1:1,-1:1], c000, s033\n"
												// c000 = c000 * s033
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return (a);
}

triVec3f* triVec3Refract(triVec3f* a, const triVec3f* b, const triVec3f* c, const triFloat eta)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.s			s010, 0 + %1\n"			// s010 = b->x
		"lv.s			s011, 4 + %1\n"			// s011 = b->y
		"lv.s			s012, 8 + %1\n"			// s012 = b->z
		"lv.s			s020, 0 + %2\n"			// s020 = c->x
		"lv.s			s021, 4 + %2\n"			// s021 = c->y
		"lv.s			s022, 8 + %2\n"			// s022 = c->z
		"lv.s			s030, %3\n"				// s030 = eta
		"vdot.t			s031, c010, c020\n"		// s031 = s010*s020 + s011*s021 + s012*s022
		"vscl.t			c010, c010, s030\n"		// c010 = c010 * s030 = *b * eta
		"vmul.s			s032, s030, s030\n"		// s032 = s030 * s030 = eta * eta
		"vmul.s			s033, s031, s031\n"		// s033 = s031 * s031 = d * d
		"vmul.s			s031, s031, s030\n"		// s031 = s031 * s030 = d * eta
		"vocp.s			s033, s033\n"			// s033 = 1.0 - s033  = 1 - d*d
		"vmul.s			s033, s032, s033\n"		// s033 = s033 * s032 = eta * eta * (1 - d*d)
		"vocp.s			s033, s033\n"			// s033 = 1.0 - s033  = 1 - eta * eta * (1 - d*d)
		"vsqrt.s		s033, s033\n"			// s033 = sqrt(s033)  = sqrt(1 - eta * eta * (1 - d*d))
		"vsub.s			s031, s031, s033\n"		// s031 = s031 - s032 = d * eta - sqrt(1 - eta * eta * (1 - d*d))
		"vscl.t			c020, c020, s031\n"		// c020 = c020 * s031
		"vadd.t			c000, c010, c020\n"		// c000 = c010 + c020
		"vdot.t			s033, c000, c000\n"		// s033 = c000 * c000
		"vcmp.t			ES, c000\n"				// CC[4] = isinfornan(s000) | isinfornan(s001) | isinfornan(s002)
		"vrsq.s			s033, s033\n"			// s033 = 1.0 / sqrt(s033)
		"vscl.t			c000[-1:1,-1:1,-1:1], c000, s033\n"
												// c000 = c000 * s033
		"vcmovt.t		c000, c000[0,0,0], 4\n"	// if (CC[4]) c000 = 0
		"sv.s			s000, 0 + %0\n"			// a->x = s000
		"sv.s			s001, 4 + %0\n"			// a->y = s001
		"sv.s			s002, 8 + %0\n"			// a->z = s002
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c), "m"(eta)
	);
	return (a);
}


triVec4f* triVec4Reflect(triVec4f* a, const triVec4f* b, const triVec4f* c)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vdot.t			s031, c000, c020\n"		// s031 = s000*s020 + s001*s021 + s002*s022
		"vfim.s			s030, -2.0\n"			// s030 = -2.0f
		"vmul.s			s032, s030, s031\n"		// s032 = s030 * s031
		"vscl.t			c020, c020, s032\n"		// c020 = c020 * s032
		"vadd.t			c000, c000, c020\n"		// c000 = c000 + c020
		"vdot.t			s033, c000, c000\n"		// s033 = c000 * c000
		"vcmp.s			EZ, s033\n"				// CC[0] = (s033==0.0f)
		"vrsq.s			s033, s033\n"			// s033 = 1.0 / sqrt(s033)
		"vcmovt.s		s033, s033[0], 0\n"		// if (CC[0]) s033 = 0
		"vscl.t			c000[-1:1,-1:1,-1:1], c000, s033\n"
												// c000 = c000 * s033
		"sv.q			c000, %0\n"				// *a  = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return (a);
}


triVec4f* triVec4Refract(triVec4f* a, const triVec4f* b, const triVec4f* c, const triFloat eta)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"lv.s			s030, %3\n"				// s030 = eta
		"vdot.t			s031, c000, c020\n"		// s031 = s000*s020 + s001*s021 + s002*s022 = d := b*c
		"vscl.t			c000, c000, s030\n"		// c000 = c000 * s030 = b * eta
		"vmul.s			s032, s030, s030\n"		// s032 = s030 * s030 = eta * eta
		"vmul.s			s033, s031, s031\n"		// s033 = s031 * s031 = d * d
		"vmul.s			s031, s031, s030\n"		// s031 = s031 * s030 = d * eta
		"vocp.s			s033, s033\n"			// s033 = 1.0 - s033  = 1 - d*d
		"vmul.s			s033, s032, s033\n"		// s033 = s033 * s032 = eta * eta * (1 - d*d)
		"vocp.s			s033, s033\n"			// s033 = 1.0 - s033  = 1 - eta * eta * (1 - d*d)
		"vsqrt.s		s033, s033\n"			// s033 = sqrt(s033)  = sqrt(1 - eta * eta * (1 - d*d)) =: phi2
		"vsub.s			s031, s031, s033\n"		// s031 = s031 - s032 = d * eta - phi2
		"vscl.t			c020, c020, s031\n"		// c020 = c020 * s031 = c * (d * eta - phi2)
		"vadd.t			c000, c000, c020\n"		// c000 = c000 + c020 = b * eta + c * (d * eta - phi2) =: v (refracted triVec)
		"vdot.t			s033, c000, c000\n"		// s033 = c000 * c000 = v * v
		"vcmp.t			ES, c000\n"				// CC[4] = isinfornan(s000) | isinfornan(s001) | isinfornan(s002)
		"vrsq.s			s033, s033\n"			// s033 = 1.0 / sqrt(s033) = 1.0 / sqrt(v*v)
		"vscl.t			c000[-1:1,-1:1,-1:1], c000, s033\n"
												// c000 = c000 * s033 = v / sqrt(v*v) = unitize(v)
		"vcmovt.t		c000, c000[0,0,0], 4\n"	// if (CC[4]) c000 = 0
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c), "m"(eta)
	);
	return (a);
}




triMat4f* triMat4Identity( triMat4f* a )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"vmidt.q		e000\n"					// e000 = 0
		"sv.q			c000,  0 + %0\n"		// pm->x = c000
		"sv.q			c010, 16 + %0\n"		// pm->y = c010
		"sv.q			c020, 32 + %0\n"		// pm->z = c020
		"sv.q			c030, 48 + %0\n"		// pm->w = c030
		".set			pop\n"					// restore assember option
		: "=m"(*a)
	);

	return (a);
}


triMat4f* triMat4Zero( triMat4f* a )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"vmzero.q		e000\n"					// e000 = 0
		"sv.q			c000,  0 + %0\n"		// pm->x = c000
		"sv.q			c010, 16 + %0\n"		// pm->y = c010
		"sv.q			c020, 32 + %0\n"		// pm->z = c020
		"sv.q			c030, 48 + %0\n"		// pm->w = c030
		".set			pop\n"					// restore assember option
		: "=m"(*a)
	);

	return (a);
}


triMat4f* triMat4Copy( triMat4f* a, const triMat4f* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000,  0 + %1\n"		// c000 = b->x
		"lv.q			c010, 16 + %1\n"		// c010 = b->y
		"lv.q			c020, 32 + %1\n"		// c020 = b->z
		"lv.q			c030, 48 + %1\n"		// c030 = b->w
		"sv.q			c000,  0 + %0\n"		// a->x = c000
		"sv.q			c010, 16 + %0\n"		// a->y = c010
		"sv.q			c020, 32 + %0\n"		// a->z = c020
		"sv.q			c030, 48 + %0\n"		// a->w = c030
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return(a);
}


triMat4f* triMat4Mul( triMat4f* a, const triMat4f* b, const triMat4f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c100,  0 + %1\n"		// c100 = b->x
		"lv.q			c110, 16 + %1\n"		// c110 = b->y
		"lv.q			c120, 32 + %1\n"		// c120 = b->z
		"lv.q			c130, 48 + %1\n"		// c130 = b->w
		"lv.q			c200,  0 + %2\n"		// c200 = c->x
		"lv.q			c210, 16 + %2\n"		// c210 = c->y
		"lv.q			c220, 32 + %2\n"		// c220 = c->z
		"lv.q			c230, 48 + %2\n"		// c230 = c->w
		"vmmul.q		e000, e200, e100\n"		// e000 = e100 * e200
		"sv.q			c000,  0 + %0\n"		// a->x = c000
		"sv.q			c010, 16 + %0\n"		// a->y = c010
		"sv.q			c020, 32 + %0\n"		// a->z = c020
		"sv.q			c030, 48 + %0\n"		// a->w = c030
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}


triMat4f* triMat4Inv( triMat4f* a, const triMat4f* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c100,  0 + %1\n"		// c100 = b->x
		"lv.q			c110, 16 + %1\n"		// c110 = b->y
		"lv.q			c120, 32 + %1\n"		// c120 = b->z
		"lv.q			c000, 48 + %1\n"		// c000 = b->w
		"vzero.t		c130\n"					// c130 = (0, 0, 0)
		"vtfm3.t		c010, m100, c000\n"		// c010 = [e100]tr * c000
		"sv.q			r100,  0 + %0\n"		// a->x = r100
		"sv.q			r101, 16 + %0\n"		// a->y = r101
		"vneg.t			c000, c010\n"			// c000 = (s010, s011, s012, s003)
		"sv.q			r102, 32 + %0\n"		// a->z = r102
		"sv.q			c000, 48 + %0\n"		// a->w = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}


triFloat triMat4Det( const triMat4f* a )
{
	triFloat v;
	__asm__ (
		".set			push\n"						// save assember option
		".set			noreorder\n"				// suppress reordering
		"lv.q			c000,  0 + %1\n"			// c000 = a->x = (a11,a21,a31,a41)
		"lv.q			c010, 16 + %1\n"			// c010 = a->y = (a12,a22,a32,a42)
		"lv.q			c020, 32 + %1\n"			// c020 = a->z = (a13,a23,a33,a43)
		"lv.q			c030, 48 + %1\n"			// c030 = a->w = (a14,a24,a34,a44)
		"vmul.t			c100, c011[X,Z,Y], c021[Y,X,Z]\n"	// c100 = (a22*a33, a23*a42, a32*a43)
		"vmul.t			c110, c001[X,Z,Y], c021[Y,X,Z]\n"	// c110 = (a21*a33, a23*a41, a31*a43)
		"vmul.t			c120, c001[X,Z,Y], c011[Y,X,Z]\n"	// c120 = (a21*a32, a22*a41, a31*a42)
		"vmul.t			c130, c001[X,Z,Y], c011[Y,X,Z]\n"	// c130 = (a21*a32, a22*a41, a31*a42)
		"vmul.t			c200, c011[Z,Y,X], c021[Y,X,Z]\n"	// c200 = (a33*a42, a23*a32, a22*a43)
		"vmul.t			c210, c001[Z,Y,X], c021[Y,X,Z]\n"	// c210 = (a33*a41, a23*a31, a21*a43)
		"vmul.t			c220, c001[Z,Y,X], c011[Y,X,Z]\n"	// c220 = (a32*a41, a22*a31, a21*a42)
		"vmul.t			c230, c001[Z,Y,X], c011[Y,X,Z]\n"	// c230 = (a32*a41, a22*a31, a21*a42)
		"vdot.t			s100, c100, c031[Z,Y,X]\n"	// c100 = (a22*a33*a44 + a23*a34*a42 + a24*a32*a43)
		"vdot.t			s110, c110, c031[Z,Y,X]\n"	// c110 = (a21*a33*a44 + a23*a34*a41 + a24*a31*a43)
		"vdot.t			s120, c120, c031[Z,Y,X]\n"	// c120 = (a21*a32*a44 + a22*a34*a41 + a24*a31*a42)
		"vdot.t			s130, c130, c021[Z,Y,X]\n"	// c130 = (a21*a32*a43 + a22*a33*a41 + a23*a31*a42)
		"vdot.t			s200, c200, c031[X,Z,Y]\n"	// c200 = (a24*a33*a42 + a23*a32*a44 + a22*a34*a43)
		"vdot.t			s210, c210, c031[X,Z,Y]\n"	// c210 = (a24*a33*a41 + a23*a31*a44 + a21*a34*a43)
		"vdot.t			s220, c220, c031[X,Z,Y]\n"	// c220 = (a24*a32*a41 + a22*a31*a44 + a21*a34*a42)
		"vdot.t			s230, c230, c021[X,Z,Y]\n"	// c230 = (a23*a32*a41 + a22*a31*a43 + a21*a33*a42)
		"vsub.q			r100, r100, r200\n"			// r100 = r100 - r200 = (s100-s200, s110-s210, s120-s220, s130-s230)
		"vmul.q			r101, r000, r100\n"			// r101 = r000 * r100 = (s000*s100, s010*s110, s020*s120, s030*s130)
		"vfad.q			s000, r101[X,-Y,Z,-W]\n"	// s000 = s101 - s111 + s121 - s131
		"sv.s			s000, %0\n"					// v = s000
		".set			pop\n"						// restore assember option
		: "=m"(v)
		: "m"(*a)
	);
	return (v);
}


triFloat triMat4Trace( const triMat4f* a )
{
	return (a->m[0] + a->m[5] + a->m[10] + a->m[15]);
}


triMat4f* triMat4Trans( triMat4f* a, const triMat4f* b )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000,  0 + %1\n"		// c000 = b->x
		"lv.q			c010, 16 + %1\n"		// c010 = b->y
		"lv.q			c020, 32 + %1\n"		// c020 = b->z
		"lv.q			c030, 48 + %1\n"		// c030 = b->w
		"sv.q			r000,  0 + %0\n"		// a->x = r000
		"sv.q			r001, 16 + %0\n"		// a->y = r010
		"sv.q			r002, 32 + %0\n"		// a->z = r020
		"sv.q			r003, 48 + %0\n"		// a->w = r030
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return(a);
}


triVec4f* triMat4Apply( triVec4f* a, const triMat4f* b, const triVec4f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c100,  0 + %1\n"		// c100 = b->x
		"lv.q			c110, 16 + %1\n"		// c110 = b->y
		"lv.q			c120, 32 + %1\n"		// c120 = b->z
		"lv.q			c130, 48 + %1\n"		// c130 = b->w
		"lv.q			c200, %2\n"				// c200 = *c
		"vtfm4.q		c000, e100, c200\n"		// c000 = e100 * c200
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}


triVec4f* triMat4Apply3( triVec4f* a, const triMat4f* b, const triVec4f* c )
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c100,  0 + %1\n"		// c100 = b->x
		"lv.q			c110, 16 + %1\n"		// c110 = b->y
		"lv.q			c120, 32 + %1\n"		// c120 = b->z
		"lv.q			c200, %2\n"				// c200 = *c
		"vmov.s			s003, s203\n"			// s003 = s203
		"vtfm3.t		c000, e100, c200\n"		// c000 = e100 * c200
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return(a);
}





triQuat* triQuatUnit(triQuat* a)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"vidt.q			c030\n"					// c030 = (0.0, 0.0, 0.0, 1.0)
		"sv.q			c030, %0\n"				// *pq  = c030
		".set			pop\n"					// restore assember option
		: "=m"(*a)
	);
	return (a);
}


triQuat* triQuatCopy(triQuat* a, const triQuat* b)
{
	__asm__ (
		".set			push\n"							// save assember option
		".set			noreorder\n"					// suppress reordering
		"lv.q			c000,  %1\n"
		"sv.q			c000,  %0\n"
		:"=m"(*a)
		:"m"(*b)
	);
	return (a);
}


triMat4f* triQuatToMatrix(triMat4f* a, const triQuat* b)
{
	__asm__ (
		".set			push\n"							// save assember option
		".set			noreorder\n"					// suppress reordering
		"lv.q			c130,  %1\n"					// c130 = *b = (x, y, z, w)
		"vmov.q			c100, c130[ W,  Z, -Y, -X]\n"	// c100 = ( w,  z, -y, -x)
		"vmov.q			c110, c130[-Z,  W,  X, -Y]\n"	// c110 = (-z,  w,  x, -y)
		"vmov.q			c120, c130[ Y, -X,  W, -Z]\n"	// c120 = ( y, -x,  w, -z)
		"vmov.q			c200, c130[ W,  Z, -Y,  X]\n"	// c200 = ( w,  z, -y,  x)
		"vmov.q			c210, c130[-Z,  W,  X,  Y]\n"	// c210 = (-z,  w,  x,  y)
		"vmov.q			c220, c130[ Y, -X,  W,  Z]\n"	// c220 = ( y, -x,  w,  z)
		"vmov.q			c230, c130[-X, -Y, -Z,  W]\n"	// c230 = (-x, -y, -z,  w)
		"vmmul.q		e000, e200, e100\n"				// e000 = e100 * e200
		"sv.q			c000,  0 + %0\n"				// a->x = c000
		"sv.q			c010, 16 + %0\n"				// a->y = c010
		"sv.q			c020, 32 + %0\n"				// a->z = c020
		"sv.q			c030, 48 + %0\n"				// a->w = c030
		".set			pop\n"							// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}


triVec4f* triQuatApply(triVec4f* a, const triQuat* b, const triVec4f* c)
{
	__asm__ (
		".set			push\n"							// save assember option
		".set			noreorder\n"					// suppress reordering
		"lv.q			c100,  %1\n"					// c100 = *b
		"lv.q			c200,  %2\n"					// c200 = *c
		"vmov.q			c110, c100[-X,-Y,-Z,W]\n"		// c110 = (-s110, -s111, -s112, s113)
		"vqmul.q		c120, c100, c200\n"				// c120 = c200 * c100
		"vqmul.q		c000, c120, c110\n"				// c000 = c120 * c110
		"sv.q			c000, %0\n"						// *a = c000
		".set			pop\n"							// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return (a);
}


triQuat* triQuatAdd(triQuat* a, const triQuat* b, const triQuat* c)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vadd.q			c000, c010, c020\n"		// c000 = c010 + c020
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return (a);
}


triQuat* triQuatSub(triQuat* a, const triQuat* b, const triQuat* c)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vsub.q			c000, c010, c020\n"		// c000 = c010 - c020
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return (a);
}


triQuat* triQuatMul(triQuat* a, const triQuat* b, const triQuat* c)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vqmul.q		c000, c010, c020\n"		// c000 = c010 * c020
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c)
	);
	return (a);
}


triFloat triQuatInnerProduct(const triQuat* a, const triQuat* b)
{
	triFloat f;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c010, %1\n"				// c010 = *a
		"lv.q			c020, %2\n"				// c020 = *b
		"vdot.q			s000, c010, c020\n"		// s000 = c010 dot c020
		"sv.s			s000, %0\n"				// f    = s000
		".set			pop\n"					// restore assember option
		: "=m"(f)
		: "m"(*a), "m"(*b)
	);
	return (f);
}


triQuat* triQuatNLerp(triQuat* a, const triQuat* b, const triQuat* c, triFloat t)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"mfc1			$8,   %3\n"				// t0   = t
		"mtv			$8,   s031\n"			// s031 = t0 = t
		"lv.q			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vscl.q			c020, c020, s031\n"
		"vocp.s			s031, s031\n"
		"vscl.q			c010, c010, s031\n"
		"vadd.q			c000, c010, c020\n"
		"sv.q			c000, %0\n"				// *a  = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c), "f"(t)
	);
	return (a);
}


triQuat* triQuatSLerp(triQuat* a, const triQuat* b, const triQuat* c, triFloat t)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"mfc1			$8,   %3\n"				// t0   = t
		"mtv			$8,   s031\n"			// s031 = t0 = t
		"lv.q			c010, %1\n"				// c010 = *b
		"lv.q			c020, %2\n"				// c020 = *c
		"vcst.s			s001, VFPU_SQRT1_2\n"	// s001 = VFPU_SQRT1_2 = 1 / sqrt(2)
		"vdot.q			s000[-1:1], c010, c020\n"	// s000 = c010 dot c020 = *pq0 dot *pq1
		"vocp.s			s030, s031\n"			// s030 = 1 - s031    = (1 - t)
		"vcmp.s			LT, s000[|x|], s001\n"	// CC[0] = |s000| < s001

		"vasin.s		s032, s000[|x|]\n"		// s032 = asin(abs(s000))
		"bvtl			0, 0f\n"				// if (CC[0]!=0) goto 0f
		"vocp.s			s032, s032\n"			// s032 = 1 - s032    = acos(abs(s000)) = angle

		"vmul.s			s001, s000, s000\n"		// s001 = s000 * s000
		"vocp.s			s001, s001\n"			// s001 = 1 - s001
		"vsqrt.s		s001, s001\n"			// s001 = sqrt(s001)
		"vasin.s		s032, s001\n"			// s032 = asin(s001)  = asin(sqrt(1-s000*s000))
												//                    = acos(abs(s000)) = angle
	"0:\n"
		"vzero.s		s001\n"					// s001 = 0
		"vfim.s			s002, 0.00005\n"		// s002 = EPSILON
		"vscl.p			c030, c030, s032\n"		// s030 = s030 * s032 = (1 - t) * angle
												// s031 = s031 * s032 = t       * angle
		"vcmp.s			LT,   s000, s001\n"		// VFPU_CC[0] = (s000 < 0)
		"vsin.t			c030, c030\n"			// s030 = sin(s030)   = sin((1 - t) * angle)
												// s031 = sin(s031)   = sin(t)      * angle)
												// s032 = sin(s032)   = sin(angle)
		"vcmovt.q		c020, c020[-x,-y,-z,-w], 0\n"	// if (VFPU_CC[0]) c020 = -c020
		"vcmp.s			LT,   s032, s002\n"		// VFPU_CC[0] = (s032 < EPSILON)
		"vrcp.s			s032, s032\n"			// s032 = 1.0f / s032 = 1 / sin(angle)
		"vscl.p			c030, c030, s032\n"		// s030 = s030 * s032 = sin((1 - t) * angle) / sin(angle)
												// s031 = s031 * s032 = sin(t       * angle) / sin(angle)
		"vscl.q			c000, c010, s030\n"		// c000 = c010 * s030
		"vscl.q			c030, c020, s031\n"		// c030 = c020 * s031
		"vadd.q			c000, c000, c030\n"		// c000 = c000 + c030
		"vcmovt.q		c000, c010, 0\n"		// if (VFPU_CC[0]) c000 = c010
		"sv.q			c000, %0\n"				// *a  = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b), "m"(*c), "f"(t)
		: "$8"
	);
	return (a);
}


triQuat* triQuatSquad(triQuat* a, const triQuat* b, const triQuat* c,
								  const triQuat* d, const triQuat* e, triFloat t)
{
	triQuat qa, qb;
	triQuatSLerp(&qa, b, c, t);
	triQuatSLerp(&qb, d, e, t);
	triQuatSLerp(a, &qa, &qb, 2*t*(1-t));
	return (a);
}


triQuat* triQuatNormalize(triQuat* a, const triQuat* b)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *b
		"vdot.q			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002 + s003*s003
		"vrsq.s			s010, s010\n"			// s011 = 1.0 / sqrt(s010)
		"vscl.q			c000, c000, s010\n"		// c000 = c000 * s010
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}


triQuat* triQuatConj(triQuat* a, const triQuat* b)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *b
		"vneg.t			c000, c000\n"			// c000 = -c000
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}


triQuat* triQuatInverse(triQuat* a, const triQuat* b)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c010, %1\n"				// c010 = *b
		"vdot.q			s020, c010, c010\n"		// s020 = c010 dot c010
		"vrcp.s			s020, s020\n"			// s020 = 1.0 / s020
		"vscl.q			c000, c010[-X,-Y,-Z,W], s020\n"
												// c000 = c010[-X,-Y,-Z,W] * s020
												// s000 = -s010 * s020
												// s001 = -s011 * s020
												// s002 = -s012 * s020
												// s003 =  s013 * s020
		"sv.q			c000, %0\n"				// *a = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "m"(*b)
	);
	return (a);
}



triQuat* triQuatFromRotate(triQuat* a, triFloat angle, const triVec4f* b)
{
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %2\n"				// c000 = *b
		"mfc1			$8,   %1\n"				// t0   = angle
		"vdot.q			s010, c000, c000\n"		// s010 = s000*s000 + s001*s001 + s002*s002 + s003*s003
		"vcst.s			s020, VFPU_1_PI\n"		// s020 = VFPU_1_PI = 1 / PI
		"mtv			$8,   s021\n"			// s021 = t0 = angle
		"vmul.s			s020, s020, s021\n"		// s020 = s020 * s021 = angle * 0.5 * (2/PI)
		"vcos.s			s003, s020\n"			// s003 = cos(s020)
		"vsin.s			s020, s020\n"			// s020 = sin(s020)
		"vrsq.s			s010, s010\n"			// s010 = 1.0 / sqrt(s010)
		"vmul.s			s010, s010, s020\n"		// s010 = s010 * s020
		"vscl.t			c000, c000, s010\n"		// c000 = c000 * s010
		"sv.q			c000, %0\n"				// *a  = c000
		".set			pop\n"					// restore assember option
		: "=m"(*a)
		: "f"(angle), "m"(*b)
		: "$8"
	);

	return (a);
}





triU32 triColor4f2RGBA8888( triColor4f* a )
{
	triU32 c;
	__asm__ (
		".set			push\n"					// save assember option
		".set			noreorder\n"			// suppress reordering
		"lv.q			c000, %1\n"				// c000 = *a
		"vsat0.q		c000,  c000\n"			// c000 = saturation to [0:1](c000)
		"viim.s			s010, 255\n"			// s010 = 255.0f
		"vscl.q			c000, c000, s010\n"		// c000 = c000 * 255.0f
		"vf2iz.q		c000, c000, 23\n"		// c000 = (int)c000 * 2^23
		"vi2uc.q		s000, c000\n"			// s000 = ((s003>>23)<<24) | ((s002>>23)<<16) | ((s001>>23)<<8) | (s000>>23)
		"mfv			%0,   s000\n"			// c    = s000
		".set			pop\n"					// restore assember option
		: "=&r"(c)
		: "m"(*a)
	);
	return (c);
}


