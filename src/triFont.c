/*
 * triFont.c: Implementation for bitmap font library
 * This file is part of the "tri Engine".
 *
 * Copyright (C) 2007 tri
 * Copyright (C) 2007 David Perry 'InsertWittyName' <tias_dp@hotmail.com>
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
 
#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "triGraphics.h"	// Needed for triEnable/Disable
#include "triFont.h"
#include "triVAlloc.h"
#include "triLog.h"
#include "triMemory.h"

triUChar triDebugFontTexture[] __attribute__((aligned(16))) = {
				0xB1,0x80,0x0A,0xE1,0xB6,0xCB,0x16,0x1B,0x0B,0x50,0xCC,0x3C,0xDC,0x0E,0xBA,0xCC,
				0x00,0xC0,0x01,0xE1,0x00,0x30,0x0D,0x00,0x00,0xE0,0x01,0xC4,0x10,0x0E,0x00,0x30,
				0x00,0x00,0x00,0xE1,0x00,0xB4,0x04,0x00,0x00,0xE1,0x00,0xE1,0x10,0x0E,0x00,0xB3,
				0x00,0x00,0x00,0xE1,0x10,0x0F,0x10,0x1B,0x1E,0xE0,0x01,0xC4,0x10,0x0E,0x92,0x08,
				0x00,0x00,0x00,0xB1,0x10,0x0B,0x00,0x80,0x05,0x50,0xCC,0x3C,0x98,0x8F,0xDF,0xCC,
				0xE1,0x00,0x10,0xCE,0xCF,0x00,0xC9,0x1E,0x0E,0x20,0x5B,0x10,0x0F,0x00,0x10,0xBF,
				0xE1,0x00,0x10,0x1E,0x0E,0x00,0x10,0x1E,0x0E,0xA7,0x01,0x10,0x0F,0x00,0x10,0x9E,
				0xF1,0xCC,0xCC,0x1E,0x0E,0x00,0x10,0x1E,0xBF,0x0D,0x00,0x10,0x0F,0x00,0x10,0x0E,
				0xA6,0xBB,0x5A,0x00,0xA0,0x0E,0xF1,0xCC,0xBC,0x10,0xB8,0xAB,0xC0,0xCC,0xFC,0x80,
				0x0D,0x00,0xC3,0x30,0x4A,0x0E,0xF1,0xCC,0x2A,0xC0,0xBB,0xAC,0x02,0x00,0x87,0xE0,
				0x04,0xC0,0x2E,0x96,0x10,0x0E,0x00,0x00,0xD6,0xF1,0x00,0x50,0x0D,0x40,0x0B,0x70,
				0x00,0x00,0xD2,0xCC,0xCC,0xAF,0x00,0x00,0xC4,0xF0,0x03,0x40,0x0C,0xC2,0x00,0xF0,
				0xAC,0xCB,0x5C,0x00,0x10,0x0E,0xA1,0xCB,0x2B,0x40,0xCC,0xBC,0x12,0x2C,0x00,0x80,
				0x00,0xC0,0x1E,0xDF,0x01,0xE1,0x20,0xCA,0xCC,0x09,0xF1,0xBC,0x6C,0x20,0xCA,0xCC,
				0x06,0x88,0x1E,0x4E,0x0B,0xE1,0xD0,0x05,0x00,0xA8,0xE1,0x00,0xD4,0xD0,0x05,0x00,
				0x4D,0x1B,0x1E,0x0E,0x97,0xE1,0xF1,0x00,0x00,0xE2,0xF1,0xCC,0x3B,0xF1,0x00,0x00,
				0xBC,0x6C,0x60,0xBB,0xBC,0x02,0x40,0xBE,0x00,0x10,0xCF,0xDC,0x08,0x10,0xC8,0xBB,
				0x02,0xC3,0xF0,0x01,0x30,0x0C,0xD0,0xD3,0x04,0x10,0x0F,0x30,0x0D,0xC0,0x06,0x00,
				0x7A,0x5C,0x60,0xCB,0xAB,0x0E,0xA6,0x40,0x0D,0x10,0xCF,0xDC,0x2A,0xF1,0x00,0x00,
				0x00,0xD3,0x00,0x00,0xB0,0x18,0xAE,0x99,0x6E,0x10,0x0F,0x00,0xD4,0xD0,0x06,0x00,
				0xBD,0x5C,0x10,0xCC,0x7C,0x90,0x08,0x00,0xE2,0x11,0xCF,0xCC,0x5C,0x10,0xC9,0xBB,
				0x19,0xF1,0xBC,0x7C,0x00,0x70,0xBC,0xAB,0xC5,0xCC,0xCF,0xCC,0xE1,0x00,0x10,0x9E,
				0xA8,0xE1,0x00,0xD3,0x00,0xF1,0x02,0x00,0x00,0x10,0x0E,0x00,0xE1,0x00,0x10,0x1E,
				0xE2,0xF1,0xDC,0x1C,0x00,0x50,0xBB,0x9B,0x03,0x10,0x0E,0x00,0xE1,0x00,0x10,0x0E,
				0x7A,0xF1,0xCC,0xBC,0x05,0xF1,0xCC,0xCC,0x11,0xCF,0xCC,0x0B,0x81,0xBC,0xAB,0x07,
				0x00,0xE1,0x00,0x10,0x8A,0xF1,0x00,0x00,0x10,0x0F,0x00,0x00,0x6C,0x00,0x00,0x00,
				0x00,0xE1,0x00,0x00,0xE2,0xF1,0xCC,0xCC,0x11,0xCF,0xCC,0x14,0x0F,0x20,0xCC,0x0C,
				0x00,0xE1,0x00,0x00,0x99,0xF1,0x00,0x00,0x10,0x0F,0x00,0x00,0x6D,0x00,0x10,0x0E,
				0x7A,0xF1,0xCC,0xCC,0x06,0xF1,0xCC,0xCC,0x11,0x0F,0x00,0x00,0x91,0xBC,0xBB,0x08,
				0x08,0x00,0xE1,0x41,0x0C,0x50,0x2F,0x00,0x1E,0xD2,0x03,0x90,0x08,0x00,0x00,0x00,
				0x2E,0x00,0x69,0x00,0x2E,0xB0,0x87,0x50,0x0A,0x10,0x4C,0x7A,0x00,0x00,0x00,0x00,
				0xA6,0x20,0x0D,0x00,0x97,0xB1,0xD0,0xB0,0x03,0x00,0xF2,0x0A,0x00,0x00,0x00,0x00,
				0xE1,0x00,0x10,0x1E,0x0E,0x00,0x20,0x1E,0x0F,0xC5,0x03,0x10,0x0F,0x00,0x10,0x0E,
				0xE1,0x00,0x10,0xCE,0xCF,0xC1,0xCB,0x16,0x0E,0x10,0x8B,0x10,0x9F,0x99,0x18,0x0E,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0xB6,0x00,0xC1,0xC4,0xCC,0xDC,0x0F,0xB1,0xCB,0x17,0x0E,0x00,0x00,0xC4,0xBC,0x09,
				0x70,0x1A,0x4C,0x00,0x00,0xB4,0x02,0xB5,0xCC,0x1E,0x9F,0xDB,0x05,0x2F,0x00,0x00,
				0x00,0xE7,0x04,0x00,0x90,0x08,0x10,0x1F,0x20,0x1E,0x0E,0x30,0x0D,0x1F,0x00,0x00,
				0x00,0xE1,0x00,0x20,0x4B,0x00,0x00,0xC9,0xAC,0x1E,0x0E,0x40,0x0C,0xC4,0xBC,0x08,
				0x00,0xE1,0x00,0xF0,0xCD,0xCC,0x0C,0x00,0x00,0x10,0xBF,0xBC,0x01,0x00,0x00,0x00,
				0xE3,0x11,0x1E,0x0E,0xA0,0xE8,0xD0,0x05,0x00,0xA8,0xE1,0x00,0x00,0xD0,0x05,0x00,
				0x00,0x10,0x1E,0x0E,0x00,0xEC,0x20,0xCA,0xCC,0x09,0xE1,0x00,0x00,0x10,0xCA,0xEC,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x90,
				0x00,0x10,0x0E,0xB5,0xCB,0x06,0xB9,0x0B,0xC1,0xCC,0x1E,0x0E,0x00,0x10,0x0B,0x10,
				0xC3,0xBC,0x0E,0xCF,0xCC,0xCC,0xCF,0x00,0x1D,0x10,0x1E,0x9F,0xDB,0x17,0x0E,0xC3,
				0x1E,0x10,0x0E,0x1F,0x00,0x10,0x0E,0x00,0x1F,0x20,0x1E,0x0E,0x20,0x1E,0x0E,0x10,
				0x1F,0x20,0x0E,0xB5,0xBB,0x19,0x0E,0x00,0xD7,0xAB,0x1E,0x0E,0x10,0x1E,0x0E,0x10,
				0xD7,0xAB,0x0E,0x00,0x00,0x10,0x0E,0x00,0xB1,0xDC,0x15,0x0E,0x10,0x1E,0x0E,0x20,
				0x98,0xE1,0x10,0x7C,0x00,0x00,0x00,0x40,0x0D,0x10,0x0E,0x00,0xF0,0x01,0x40,0x0D,
				0x07,0xE1,0x00,0xA0,0x0A,0x91,0xBB,0xBC,0x04,0x10,0x0E,0x00,0x50,0xCC,0xCC,0x03,
				0xBC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x1B,0x0E,0x00,0x00,0xE1,0xF1,0xBA,0x8D,0xBA,0x7D,0xF1,0xBA,0x7D,0x50,0xCC,0x3C,
				0x1E,0x0E,0xB2,0x06,0xE1,0xE1,0x00,0xE2,0x00,0xE2,0xE1,0x00,0xE2,0xF0,0x01,0xD4,
				0x1E,0x6E,0x1B,0x00,0xE1,0xE1,0x00,0xE1,0x00,0xE1,0xE1,0x00,0xE1,0xF0,0x01,0xD4,
				0x1E,0x6F,0x4C,0x00,0xE1,0xE1,0x00,0xE1,0x00,0xE1,0xE1,0x00,0xE1,0x50,0xCC,0x3C,
				0x1E,0x0E,0xA1,0x08,0xE1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0xD0,0xB3,0x04,0x00,0xE1,0x58,0x90,0xD7,0x00,0x20,0x3C,0x7A,0x00,0x00,0x00,0x00,
				0x40,0xBE,0x00,0x00,0xA0,0x0E,0x20,0x6F,0x00,0xC2,0x02,0xA0,0x08,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0xF1,0xBA,0x4D,0x50,0xCC,0xEC,0x9F,0x8B,0x90,0xBC,0x2B,0x10,0x00,0xE1,0x00,0xE1,
				0xE1,0x00,0xD3,0xF0,0x01,0xE1,0x0F,0x00,0xE0,0x46,0x02,0xF0,0x00,0xE1,0x00,0xE1,
				0xE1,0x00,0xD4,0xF0,0x01,0xE2,0x0F,0x00,0x00,0x53,0xB9,0xFC,0xCC,0xF1,0x00,0xE2,
				0xF1,0xCA,0x3C,0x60,0xBD,0xE9,0x0F,0x00,0xA1,0xBB,0x6C,0xF0,0x00,0x90,0xBC,0xEA,
				0xE1,0x00,0x00,0x00,0x00,0xE1,0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0xB5,0x00,0xC3,0x50,0x0A,0xF2,0x02,0x4B,0xD3,0x03,0x89,0x50,0x0B,0x40,0x0C,0xCC,
				0xC0,0x04,0x3C,0x00,0x0E,0x6A,0x1A,0x0D,0x10,0xBC,0x06,0x00,0x6A,0xD0,0x02,0x00,
				0x30,0x5D,0x09,0x00,0x89,0x0A,0x8B,0x07,0x10,0xCB,0x06,0x00,0xD1,0x6B,0x00,0xA1,
				0x00,0xE9,0x01,0x00,0xF2,0x03,0xF4,0x01,0xC3,0x02,0x8A,0x00,0x70,0x0B,0x00,0xDE,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xD1,0x01,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0xDC,
				0xEC,0x0E,0x40,0xA8,0xCB,0x5B,0x00,0x10,0x28,0x07,0x00,0xA0,0x00,0xA0,0xCC,0x07,
				0xA6,0x01,0x48,0xC5,0xBC,0xDF,0x06,0xB7,0xB8,0x78,0x70,0xB5,0x85,0xF1,0x20,0x0E,
				0x06,0x00,0x0E,0x1F,0x10,0x3E,0x0D,0x90,0x90,0x00,0xE0,0xA5,0x00,0x80,0xCC,0x85,
				0xCC,0x0C,0x2F,0x4E,0x51,0x2E,0xCC,0xEC,0xDC,0x0C,0x00,0xC4,0x98,0x00,0x00,0x95,
				0x00,0x00,0xD6,0xA8,0x7A,0x7A,0x02,0x36,0x18,0x00,0x80,0xEB,0x6B,0x00,0x20,0x0B,
				0x00,0x00,0x20,0xC8,0xBB,0x0A,0x00,0x00,0x00,0x00,0x00,0xA0,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xA0,0x00,0x00,0x00,0x00,
				0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0xC1,0x01,0x00,0x00,0xD6,0x05,0x00,0xC9,0xCC,0x08,0x00,0x51,0xE2,0x51,0x00,0x10,
				0x3B,0x00,0x00,0x70,0x09,0x5A,0x00,0x4D,0x61,0x0A,0x00,0x30,0xFC,0x2B,0xC0,0xCC,
				0x85,0xCC,0x06,0x87,0x00,0x90,0x06,0x87,0xBA,0xE2,0x00,0x51,0xE2,0x51,0x00,0x10,
				0xF1,0x30,0x0E,0x00,0x00,0x00,0x10,0x1F,0x10,0x99,0x00,0x00,0x00,0x00,0x00,0x10,
				0xA0,0x86,0x07,0x00,0x00,0x00,0x00,0xC7,0xBC,0x87,0x09,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xBA,0x00,0x00,0x00,
				0x0E,0x00,0xCC,0x0C,0xB0,0x04,0x97,0xF0,0x9C,0xCB,0x0E,0x00,0xC8,0x0A,0x00,0x00,
				0xCF,0xCC,0x00,0x00,0x6A,0x00,0x90,0xF7,0x00,0x10,0x0E,0x30,0x0D,0x00,0x00,0x00,
				0x0E,0x00,0x00,0x00,0x0F,0x00,0x20,0xFD,0x00,0x10,0x1E,0x8D,0x00,0x00,0x00,0x00,
				0x0E,0x00,0x00,0x00,0x0F,0x00,0x20,0xFD,0x00,0x10,0x0E,0x60,0x0B,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x6A,0x00,0x90,0xF7,0x00,0x10,0x0E,0x10,0x0F,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0xB0,0x04,0x97,0xF0,0x9C,0xCB,0x0E,0x00,0xC7,0x0A,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0xC1,0x6C,0x00,0x00,0x00,0x20,0x02,0x00,0x00,0x00,0x00,0x4A,0xA4,0x00,0x00,0xE1,
				0x00,0xE1,0x01,0x00,0x83,0x8A,0xA8,0x38,0x00,0x00,0x40,0x09,0xA0,0x04,0x00,0xE1,
				0x00,0x10,0xBA,0xCA,0x16,0x00,0x00,0x61,0xAC,0x00,0xC0,0x01,0x10,0x0C,0x00,0xE1,
				0x00,0xD0,0x03,0x61,0x99,0x05,0x50,0x99,0x16,0x00,0x59,0x00,0x00,0x86,0x00,0xE1,
				0x00,0xE2,0x00,0x00,0x00,0x94,0x49,0x00,0x00,0x30,0x0B,0x00,0x00,0xB0,0x02,0xE1,
				0xC1,0x5C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x02,0x00,0x00,0x20,0x0C,0xE1,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x40,0xBB,0x03,0xB2,0x70,0x08,0xE1,0xE1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0xC0,0x00,0xB5,0x4C,0x00,0x00,0xE1,0xE1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

triFont triDebugFont = {
	.texSize = 128,
	.texHeight = 32,
	.texture = triDebugFontTexture,
	.location = TRI_RAM,
	.map = {	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x03, 0x5B, 0x46, 0x47, 0x48, 0x4A, 0x00, 0x4E, 0x4F, 0x4B, 0x4C, 0x02, 0x4D, 0x01, 0x56, 
				0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x05, 0x06, 0x54, 0x00, 0x55, 0x04, 
				0x45, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 
				0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x50, 0x57, 0x51, 0x49, 0x00, 
				0x5A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 
				0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x52, 0x58, 0x53, 0x59, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	.glyph = {	{ 0, 0, 0, 0, 1, -6, 4, 0 },
				{ 0, 0, 2, 1, 1, -5, 3, 0 },
				{ 2, 0, 4, 2, 1, -5, 4, 0 },
				{ 6, 0, 2, 5, 2, -1, 5, 0 },
				{ 8, 0, 5, 5, 1, -1, 6, 0 },
				{ 13, 0, 2, 4, 2, -2, 5, 0 },
				{ 15, 0, 3, 5, 2, -2, 5, 0 },
				{ 18, 0, 6, 5, 1, -1, 7, 0 },
				{ 24, 0, 4, 5, 2, -1, 6, 0 },
				{ 28, 0, 5, 5, 2, -1, 7, 0 },
				{ 33, 0, 5, 5, 2, -1, 7, 0 },
				{ 38, 0, 6, 5, 1, -1, 6, 0 },
				{ 44, 0, 6, 5, 1, -1, 7, 0 },
				{ 50, 0, 7, 5, 1, -1, 8, 0 },
				{ 57, 0, 5, 5, 2, -1, 7, 0 },
				{ 62, 0, 6, 5, 1, -1, 7, 0 },
				{ 68, 0, 7, 5, 1, -1, 8, 0 },
				{ 75, 0, 8, 5, 1, -1, 7, 0 },
				{ 83, 0, 7, 5, 1, -1, 7, 0 },
				{ 90, 0, 8, 5, 1, -1, 8, 0 },
				{ 98, 0, 8, 5, 1, -1, 9, 0 },
				{ 106, 0, 7, 5, 1, -1, 7, 0 },
				{ 113, 0, 6, 5, 1, -1, 6, 0 },
				{ 119, 0, 8, 5, 1, -1, 9, 0 },
				{ 0, 5, 7, 5, 1, -1, 8, 0 },
				{ 7, 5, 3, 5, 2, -1, 5, 0 },
				{ 10, 5, 5, 5, 0, -1, 5, 0 },
				{ 15, 5, 8, 5, 1, -1, 7, 0 },
				{ 23, 5, 6, 5, 1, -1, 6, 0 },
				{ 29, 5, 8, 5, 1, -1, 9, 0 },
				{ 37, 5, 7, 5, 1, -1, 8, 0 },
				{ 44, 5, 8, 5, 1, -1, 9, 0 },
				{ 52, 5, 6, 5, 1, -1, 6, 0 },
				{ 58, 5, 8, 6, 1, -1, 9, 0 },
				{ 66, 5, 8, 5, 1, -1, 7, 0 },
				{ 74, 5, 7, 5, 1, -1, 8, 0 },
				{ 81, 5, 7, 5, 1, -1, 7, 0 },
				{ 88, 5, 7, 5, 1, -1, 8, 0 },
				{ 95, 5, 8, 5, 1, -1, 7, 0 },
				{ 103, 5, 11, 5, 1, -1, 11, 0 },
				{ 114, 5, 8, 5, 1, -1, 7, 0 },
				{ 0, 11, 7, 5, 1, -1, 7, 0 },
				{ 7, 11, 6, 5, 2, -1, 8, 0 },
				{ 13, 11, 6, 4, 1, -2, 7, 0 },
				{ 19, 11, 6, 5, 1, -1, 7, 0 },
				{ 25, 11, 6, 4, 1, -2, 6, 0 },
				{ 31, 11, 6, 5, 1, -1, 7, 0 },
				{ 37, 11, 6, 4, 1, -2, 7, 0 },
				{ 43, 11, 4, 5, 1, -1, 4, 0 },
				{ 47, 11, 6, 5, 1, -2, 7, 0 },
				{ 53, 11, 6, 5, 1, -1, 7, 0 },
				{ 59, 11, 2, 5, 1, -1, 3, 0 },
				{ 61, 11, 4, 6, 0, -1, 4, 0 },
				{ 65, 11, 7, 5, 1, -1, 6, 0 },
				{ 72, 11, 2, 5, 1, -1, 3, 0 },
				{ 74, 11, 10, 4, 1, -2, 11, 0 },
				{ 84, 11, 6, 4, 1, -2, 7, 0 },
				{ 90, 11, 6, 4, 1, -2, 6, 0 },
				{ 96, 11, 6, 5, 1, -2, 7, 0 },
				{ 102, 11, 6, 5, 1, -2, 7, 0 },
				{ 108, 11, 4, 4, 2, -2, 5, 0 },
				{ 112, 11, 6, 4, 1, -2, 6, 0 },
				{ 118, 11, 4, 6, 1, 0, 4, 0 },
				{ 122, 11, 6, 4, 1, -2, 7, 0 },
				{ 0, 17, 7, 4, 1, -2, 6, 0 },
				{ 7, 17, 9, 4, 1, -2, 9, 0 },
				{ 16, 17, 7, 4, 1, -2, 6, 0 },
				{ 23, 17, 7, 5, 1, -2, 6, 0 },
				{ 30, 17, 5, 4, 1, -2, 6, 0 },
				{ 35, 17, 10, 6, 1, -1, 11, 0 },
				{ 45, 17, 7, 5, 2, -1, 9, 0 },
				{ 52, 17, 6, 7, 1, -1, 7, 0 },
				{ 58, 17, 11, 5, 1, -1, 12, 0 },
				{ 69, 17, 8, 3, 1, -1, 9, 0 },
				{ 77, 17, 9, 5, 1, -1, 8, 0 },
				{ 86, 17, 7, 3, 1, -1, 7, 0 },
				{ 93, 17, 7, 4, 2, -2, 9, 0 },
				{ 100, 17, 3, 1, 2, -3, 5, 0 },
				{ 103, 17, 4, 6, 1, -1, 5, 0 },
				{ 107, 17, 4, 6, 1, -1, 5, 0 },
				{ 111, 17, 3, 6, 2, -1, 5, 0 },
				{ 114, 17, 3, 6, 2, -1, 5, 0 },
				{ 117, 17, 6, 6, 1, -1, 7, 0 },
				{ 0, 24, 6, 6, 1, -1, 7, 0 },
				{ 6, 24, 6, 5, 2, -1, 8, 0 },
				{ 12, 24, 6, 5, 2, -1, 8, 0 },
				{ 18, 24, 6, 6, 0, -1, 5, 0 },
				{ 24, 24, 6, 6, 1, -1, 5, 0 },
				{ 30, 24, 2, 6, 2, -1, 5, 0 },
				{ 32, 24, 8, 2, 1, -3, 9, 0 },
				{ 40, 24, 4, 1, 2, 0, 7, 0 },
				{ 44, 24, 4, 3, 1, -1, 5, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0 }
			},
	.fontHeight = 6
};


typedef struct {
	triChar		magic[8];	// "triFont "
	triUInt		size;
	triUInt		height;
	triUInt		location;
	triUChar map[256]; /**<  Character map */
	Glyph glyph[256]; /**<  Character glyphs */
	triSInt	fontHeight;
	triUInt	dataSize;
} triFontHeader;

static const triSInt TRI_FONT_TEXTURE_MIN_SIZE = 64;
static const triSInt TRI_FONT_TEXTURE_MAX_SIZE = 512;

static const triChar *TRI_FONT_CHARSET =
	" .,!?:;"
    	"0123456789"
    	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    	"abcdefghijklmnopqrstuvwxyz"
    	"@#$%^&*+-()[]{}<>/\\|~`\""
;

// Store the clut on the framebuffer's first (or second in 16bit mode) line's padding area
static triUInt* clut = (triUInt*)(0x44000000 + 480*4);
//static triUInt __attribute__((aligned(16))) clut[16];

triFont* triActiveFont = &triDebugFont;

typedef struct GlyphInfo
{
	struct GlyphInfo *next, *prev;
	triUInt c;
	Glyph glyph;
} GlyphInfo;

GlyphInfo glyphList =
{
	.next = &glyphList,
	.prev = &glyphList,
};

// Return next power of 2
static triS32 next_pow2(triU32 w)
{
	w -= 1;		// To not change already power of 2 values
	w |= (w >> 1);
	w |= (w >> 2);
	w |= (w >> 4);
	w |= (w >> 8);
	w |= (w >> 16);
	return(w+1);
}

static triSInt triFontSwizzle(triFont *font)
{
	triSInt byteWidth = font->texSize/2;
	triSInt textureSize = font->texSize*font->texHeight/2;
	triSInt height = textureSize / byteWidth;

	// Swizzle the texture
	triSInt rowBlocks = (byteWidth / 16);
	triSInt rowBlocksAdd = (rowBlocks-1)*128;
	triUInt blockAddress = 0;
	triUInt *src = (triUInt*) font->texture;
	static triUChar *tData = 0;

	if(font->location)
		tData = (triUChar*) triVAlloc(textureSize);
	if(!tData)
		tData = (triUChar*) triMalloc(textureSize);
	
	if(!tData)
		return 0;

	triSInt j;

	for(j = 0; j < height; j++, blockAddress += 16)
	{
		triUInt *block;
		if (font->location)
			block = (triUInt*)((triUInt)&tData[blockAddress]|0x40000000);
		else
			block = (triUInt*)&tData[blockAddress];
		triSInt i;

		for(i=0; i < rowBlocks; i++)
		{
			*block++ = *src++;
			*block++ = *src++;
			*block++ = *src++;
			*block++ = *src++;
			block += 28;
		}

		if((j&0x7) == 0x7)
			blockAddress += rowBlocksAdd;
	}
	sceKernelDcacheWritebackRange(tData,textureSize);
	triFree(font->texture);

	font->texture = tData;
	
	return 1;
}

#ifdef TRI_SUPPORT_FT
triFont* triFontLoad(const triChar *filename, triUInt fontSize, enum triFontSizeType fontSizeType, triUInt textureSize, enum triFontLocation location)
{
	FT_Library library;
	FT_Face face;
	FT_GlyphSlot slot;
	GlyphInfo *gp;
	GlyphInfo gi[256];
	triSInt n, count, charCount;
	triSInt xx, yy;

	charCount = strlen(TRI_FONT_CHARSET);
	count = charCount;

	textureSize = next_pow2( textureSize );	// Make sure it is really a pow2
	if((textureSize < TRI_FONT_TEXTURE_MIN_SIZE) || (textureSize > TRI_FONT_TEXTURE_MAX_SIZE))
	{
		#ifdef _DEBUG_LOG
		triLogError("triFont: Error, textureSize out of bounds: %s\r\n", filename);
		#endif
		return NULL;
	}

	triFont* font = (triFont*) triMalloc(sizeof(triFont));
	
	if(!font)
	{
		#ifdef _DEBUG_LOG
		triLogError("triFont: Error allocating font: %s\r\n", filename);
		#endif
		return NULL;
	}
	
	font->texSize = textureSize;
	font->location = location;

	if(FT_Init_FreeType(&library))
	{
		#ifdef _DEBUG_LOG
		triLogError("triFont: Freetype init failure %s\r\n", filename);
		#endif
		triFree(font);
		return NULL;
	}

	if(FT_New_Face(library, filename, 0, &face))
	{
		#ifdef _DEBUG_LOG
		triLogError("triFont: Freetype cannot load font %s\r\n", filename);
		#endif
		triFree(font);
		return NULL;
	}
	
	if(fontSizeType == TRI_FONT_SIZE_PIXELS)
	{
		if(FT_Set_Pixel_Sizes(face, fontSize, 0))
		{
			#ifdef _DEBUG_LOG
			triLogError("triFont: Freetype cannot set to fontSize: %s, %d\r\n", filename, fontSize);
			#endif
			triFree(font);
			return NULL;
		}
	}
	else
	{
		if(FT_Set_Char_Size(face, fontSize * 64, 0, 100, 0))
		{
			#ifdef _DEBUG_LOG
			triLogError("triFont: Freetype cannot set to fontSize: %s, %d\r\n", filename, fontSize);
			#endif
			triFree(font);
			return NULL;
		}
	}

	slot = face->glyph;

	triChar minLeft = 127, maxTop = -127;
	// Assemble metrics and sort by fontSize
	for(n = 0; n < count; n++)
	{
		if(FT_Load_Char(face, TRI_FONT_CHARSET[n], FT_LOAD_RENDER))
		{
			#ifdef _DEBUG_LOG
			triLogError("triFont: Cannot load character: %s, '%c'\r\n", filename, TRI_FONT_CHARSET[n]);
			#endif
			triFree(font);
			return NULL;
		}

		gi[n].c = TRI_FONT_CHARSET[n];
		gi[n].glyph.x = 0;
		gi[n].glyph.y = 0;
		gi[n].glyph.width = slot->bitmap.width;
		gi[n].glyph.height = slot->bitmap.rows;
		gi[n].glyph.top = slot->bitmap_top;
		gi[n].glyph.left = slot->bitmap_left;
		gi[n].glyph.advance = slot->advance.x / 64;
		gi[n].glyph.unused = 0;

		if (minLeft>slot->bitmap_left) minLeft=slot->bitmap_left;
		if (maxTop<slot->bitmap_top) maxTop=slot->bitmap_top;
		// Find a good fit
		gp = glyphList.next;

		while((gp != &glyphList) && (gp->glyph.height > gi[n].glyph.height))
		{
			gp = gp->next;
		}

		gi[n].next = gp;
		gi[n].prev = gp->prev;
		gi[n].next->prev = gi;
		gi[n].prev->next = gi;
	}

	triSInt x = 0;
	triSInt y = 0;
	triSInt ynext = 0;
	triSInt used = 0;

	count = 0;
	memset(font->map, 255, 256);

	font->texture = (triUChar*) triMalloc(textureSize * textureSize / 2);
	
	if(!font->texture)
	{
		#ifdef _DEBUG_LOG
		triLogError("triFont: Error allocating font texture: %s\r\n", filename);
		#endif
		triFree(font);
		return NULL;
	}

	memset(font->texture, 0, textureSize * textureSize / 2);

	font->fontHeight = 0;
	for(n = 0; n < charCount; n++)
	{
		if(FT_Load_Char(face, gi[n].c, FT_LOAD_RENDER))
		{
			#ifdef _DEBUG_LOG
			triLogError("triFont: Cannot load character: %s, '%c'\r\n", filename, gi[n].c);
			#endif
			triFontUnload(font);
			return NULL;
		}
		
		if((x + gi[n].glyph.width) > textureSize)
		{
			y += ynext;
			x = 0;
		}

		if(gi[n].glyph.height > ynext)
			ynext = gi[n].glyph.height;

		if((y + ynext) > textureSize)
		{
			#ifdef _DEBUG_LOG
			triLogError("triFont: Error out of space in texture: %s\r\n", filename);
			#endif
			triFontUnload(font);
			return NULL;
		}

		font->map[gi[n].c] = count++;
		gi[n].glyph.x = x;
		gi[n].glyph.y = y;
		// Normalize font positions, so that printing at 0,0 will print the font completely
		gi[n].glyph.left -= minLeft;
		gi[n].glyph.top -= maxTop;

		if((triS32)(gi[n].glyph.height+gi[n].glyph.top) > font->fontHeight)
			font->fontHeight = (gi[n].glyph.height+gi[n].glyph.top);
		
		triU32 slotoffs = 0;
		triU32 texoffs = (y*textureSize)/2;
		triU8* slotbuf = slot->bitmap.buffer;
        for(yy = 0; yy < gi[n].glyph.height; yy++)
		{
			xx = 0;
			if (x & 1)
			{
				font->texture[(x>>1) + texoffs] |= (slotbuf[slotoffs] & 0xF0);
				xx++;
			}
			for(; xx < gi[n].glyph.width; xx+=2)
			{
				if (xx+1 < gi[n].glyph.width)
					font->texture[((x+xx)>>1) + texoffs] = (slotbuf[slotoffs + xx] >> 4)
													| (slotbuf[slotoffs + xx + 1] & 0xF0);
				else
					font->texture[((x+xx)>>1) + texoffs] = (slotbuf[slotoffs + xx] >> 4);
			}
			texoffs += textureSize>>1;
			slotoffs += slot->bitmap.width;
		}

		x += gi[n].glyph.width;

		used += (gi[n].glyph.width * gi[n].glyph.height);
	}

	font->texHeight = (y + ynext + 7)&~7;
	if (font->texHeight > font->texSize) font->texHeight = font->texSize;
	
	/* redirect any undefined characters to ' ' */
	for(n = 0; n < 256; n++)
	{
		if(font->map[n] == 255)
			font->map[n] = font->map[' '];
	}

    for(n = 0; n < charCount; n++)
	{
		memcpy(&font->glyph[n], &gi[n].glyph, sizeof(gi[n].glyph));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);
	
	if(!triFontSwizzle(font))
	{
		triFontUnload(font);
		#ifdef _DEBUG_LOG
		triLogError("triFont: Error swizzling font: %s\r\n", filename);
		#endif
		return NULL;
	}

	return font;
}
#endif


void triFontUnload(triFont *font)
{
	if(!font)
		return;
	
	if(font->texture)
	{
		if(font->location)
			triVFree(font->texture);
		else
			triFree(font->texture);
	}		

	triFree(font);
}



static triVoid unswizzle_fast(const triU8* out, const triU8* in, const triS32 width, const triS32 height)
{
	triS32 blockx, blocky;
	triS32 j;
	
	triS32 width_blocks = (width / 16);
	triS32 height_blocks = (height / 8);
	
	triS32 dst_pitch = (width-16)/4;
	triS32 dst_row = width * 8;
	
	triU32* src = (triU32*)in;
	triU8* ydst = (triU8*)out;
	sceKernelDcacheWritebackAll();
	for (blocky = 0; blocky < height_blocks; ++blocky)
	{
		triU8* xdst = ydst;
		for (blockx = 0; blockx < width_blocks; ++blockx)
		{
			triU32* dst;
			if ((triU32)out <= 0x04200000)
				dst = (triU32*)((triU32)xdst | 0x40000000);
			else
				dst = (triU32*)xdst;
			for (j = 0; j < 8; ++j)
			{
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				dst += dst_pitch;
			}
			xdst += 16;
		}
		ydst += dst_row;
	}
}

#if 0
void triFontSaveC(triFont *font, triChar *filename)
{
	if(!font)
		return;
	if (!filename)
		return;
	FILE* fp = fopen(filename, "w");
	char buffer[256];
	sprintf(buffer,"triUChar triMyFontTexture[] = {\n\t\t\t\t");
	fwrite(buffer,strlen(buffer),1,fp);
	int i, size = font->texSize*font->texHeight/2;
	for (i=0;i<size;i++)
	{
		sprintf(buffer,"0x%02X%s",font->texture[i],(i<size-1?",":"};\n"));
		fwrite(buffer,strlen(buffer),1,fp);
		if ((i&15)==15)
		{
			if (i==size-1)
				fwrite("\n",1,1,fp);
			else
				fwrite("\n\t\t\t\t",5,1,fp);
		}
	}
	sprintf(buffer,"\ntriFont triMyFont = {\n	.texSize = %i,\n	.texHeight = %i,\n	.texture = triMyFontTexture,\n	.location = TRI_RAM,\n	.map = {	", font->texSize, font->texHeight);
	fwrite(buffer,strlen(buffer),1,fp);
	for (i=0;i<256;i++)
	{
		sprintf(buffer,"0x%02X%s",font->map[i],(i<255?", ":"},"));
		fwrite(buffer,strlen(buffer),1,fp);
		if ((i&15)==15)
		{
			if (i==255)
				fwrite("\n",1,1,fp);
			else
				fwrite("\n\t\t\t\t",5,1,fp);
		}
	}
	sprintf(buffer, "	.glyph = {	" );
	fwrite(buffer,strlen(buffer),1,fp);
	for (i=0;i<256;i++)
	{
		Glyph* g = &header.glyph[i];
		sprintf(buffer,"{ %i, %i, %i, %i, %i, %i, %i, %i }%s", g->x, g->y, g->width, g->height, g->left, g->top, g->advance, g->unused, (i==255?"\n\t\t\t},\n":",\n\t\t\t\t") );
		fwrite(buffer,strlen(buffer),1,fp);
	}
	sprintf(buffer,"	.fontHeight = %i\n};\n", font->fontHeight);
	fwrite(buffer,strlen(buffer),1,fp);
	fclose(fp);
}
#endif

void triFontSaveTRF(triFont *font, triChar *filename)
{
	if(!font)
		return;
	if (!filename)
		return;
	triFontHeader header;
	strncpy(header.magic,"triFont ",8);
	header.size = font->texSize;
	header.height = font->texHeight;
	header.location = font->location;
	header.fontHeight = font->fontHeight;
	header.dataSize = font->texSize * font->texHeight / 2;
	memcpy(header.map, font->map, 256*sizeof(triUChar));
	memcpy(header.glyph, font->glyph, 256*sizeof(Glyph));

	FILE*fp = fopen(filename, "wb");
	if (!fp) {
		triLogError("ERROR: Could not open file '%s'.\n", filename);
		return;
	}
	fwrite( &header, sizeof(triFontHeader), 1, fp );
	fwrite( font->texture, header.dataSize, 1, fp );
	fclose( fp );
}

triFont* triFontLoadTRF(const triChar *filename)
{
	if (!filename)
		return 0;
	triFont* font = triMalloc(sizeof(triFont));
	if (!font)
		return 0;
	triFontHeader header;

	FILE* fp = fopen(filename, "rb");
	if (!fp) {
		triFree(font);
		triLogError("ERROR: Could not open file '%s'.\n", filename);
		return 0;
	}
	fread( &header, sizeof(triFontHeader), 1, fp );
	if (strncmp(header.magic,"triFont ",8)!=0)
	{
		fclose(fp);
		triFree(font);
		triLogError("ERROR: Not a triFont file '%s'.\n", filename);
		return 0;
	}
	font->texSize = header.size;
	font->texHeight = header.height;
	font->location = TRI_RAM;
	font->fontHeight = header.fontHeight;
	font->texture = triMalloc(header.dataSize);
	memcpy(font->map, header.map, 256*sizeof(triUChar));
	memcpy(font->glyph, header.glyph, 256*sizeof(Glyph));
	fread( font->texture, header.dataSize, 1, fp );
	fclose( fp );
	
	return font;
}


void triFontMakeMono( triFont* font, triS32 width )
{
	if (!font)
	{
		font = &triDebugFont;
	}
	
	if (width<=0)
	{
		int i = 0;
		int max = 0;
		for (;i<256;i++)
		{
			if (font->glyph[i].width>max) max = font->glyph[i].width;
		}
		width += max;
	}
	
	int i = 0;
	for (;i<256;i++)
	{
		Glyph* g = &font->glyph[i];
		if (g->advance>0)
		{
			triS32 xoff = (width - g->width)/2;
			g->left += xoff;
			g->advance = width;
		}
	}
}


triBool triFontInit(void)
{
	triSInt n;

	for(n = 0; n < 16; n++)
	{
		clut[n] = ((n*17) << 24) | 0xffffff;
	}
	
	return 1;
}

void triFontShutdown(void)
{
	//Nothing yet
}

void triFontActivate(triFont *font)
{
	if(!font)
	{
		font = &triDebugFont;
	}

	sceGuClutMode(GU_PSM_8888, 0, 255, 0);
	sceGuClutLoad(16 / 8, clut);

	sceGuEnable(GU_TEXTURE_2D);
	sceGuTexMode(GU_PSM_T4, 0, 0, 1);
	sceGuTexImage(0, font->texSize, next_pow2(font->texHeight), font->texSize, font->texture);
	sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
	sceGuTexOffset(0.0f, 0.0f);
	sceGuTexWrap(GU_REPEAT, GU_REPEAT);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	triActiveFont = font;
}

triSInt triFontPrintf(triFloat x, triFloat y, triUInt color, const triChar *text, ...)
{
	triChar buffer[256];
	va_list ap;
	
	va_start(ap, text);
	vsnprintf(buffer, 256, text, ap);
	va_end(ap);
	buffer[255] = 0;
	
	return triFontPrint(x, y, color, buffer);
}

triSInt triFontPrint(triFloat x, triFloat y, triUInt color, const triChar *text)
{
	if(!triActiveFont)
	{
		#ifdef _DEBUG_LOG
		triLogError("triFont: Error printing font, not a valid triFont struct\r\n");
		#endif
		triActiveFont = &triDebugFont;
	}

	typedef struct
	{
		triU16 u, v;
		//triUInt c;
		triS16 x, y, z;
	} FontVertex;

	triSInt i, length;
	FontVertex *v, *v0, *v1;
	
	if((length = strlen(text)) == 0)
		return 0;
	
	v = sceGuGetMemory(sizeof(FontVertex) * 2 * length);
	v0 = v;
	
	triFloat xstart = x;
	triSInt max = 0;
	sceGuColor( color );
	for(i = 0; i < length; i++)
	{
		if (text[i]=='\n')
		{
			if (max<x) max = (triSInt)x;
			x = xstart;
			y += triActiveFont->fontHeight+3;
		}
		else
		{
			Glyph *glyph = triActiveFont->glyph + triActiveFont->map[text[i] & 0xff];
			
			v1 = v0+1;
			
			v0->u = glyph->x;
			v0->v = glyph->y;
			v0->x = (triU16)x + glyph->left;
			v0->y = (triU16)y - glyph->top;
			v0->z = 0;
	
			v1->u = glyph->x + glyph->width;
			v1->v = glyph->y + glyph->height;
			v1->x = v0->x + glyph->width;
			v1->y = v0->y + glyph->height;
			v1->z = 0;
	
			v0 += 2;
			x += glyph->advance;
		}
	}
	if (max<x) max = x;

	triDisable(TRI_DEPTH_TEST);
	triDisable(TRI_DEPTH_MASK);
	sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D, length * 2, 0, v);
	triEnable(TRI_DEPTH_MASK);	// Need triEnable calls to make sure depthbuffer is available
	triEnable(TRI_DEPTH_TEST);
	
	return max;
}


triSInt triFontMeasureText(const triChar *text)
{
	if(!triActiveFont)
	{
		#ifdef _DEBUG_LOG
		triLogError("triFont: Error measuring font text, not a valid triFont struct\r\n");
		#endif
		triActiveFont = &triDebugFont;
	}

	triSInt x = 0;
	triSInt max = 0;
	while(*text)
	{
		if (*text=='\n')
		{
			if (max<x) max = x;
			x = 0;
		}
		else
		{
			Glyph *glyph = triActiveFont->glyph + triActiveFont->map[*text++ & 0xff];
			x += glyph->advance;
		}
	}
	if (max<x) max = x;
	
	return max;
}

