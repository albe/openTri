/*
 * triFont.h: Header for bitmap font library
 * This file is part of the "tri Engine".
 *
 * Copyright (C) 2007 tri
 * Copyright (C) 2007 David Perry 'InsertWittyName' <tias_dp@hotmail.com>
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

#ifndef __TRIFONT_H__
#define __TRIFONT_H__

/** @defgroup triFont Font Library
 *  @{
 */

#ifdef TRI_SUPPORT_FT
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#include "triTypes.h"


enum triFontLocation
{
	TRI_FONT_RAM = 0,
	TRI_FONT_VRAM,
};

/**
 * A Glyph struct
 *
 * @note This is used internally by ::triFont and has no other relevance.
 */
typedef struct Glyph
{
    triU16 x;
    triU16 y;
    triUChar width;
    triUChar height;
    triChar left;
    triChar top;
    triChar advance;
    triUChar unused;
} Glyph;

/**
 * A Font struct
 */
typedef struct
{
	triUInt texSize; /**<  Texture size (power2) */
	triUInt texHeight; /**<  Texture height (power2) */
	triUChar *texture; /**<  The bitmap data */
	enum triFontLocation location; /**<  Either in RAM or VRAM */
	triUChar map[256]; /**<  Character map */
	Glyph glyph[256]; /**<  Character glyphs */
	triSInt	fontHeight;
} triFont;

enum triFontSizeType
{
	TRI_FONT_SIZE_PIXELS = 0,
	TRI_FONT_SIZE_POINTS
};

/**
 * Initialise the Font library
 *
 * @returns true on success.
 */
triBool triFontInit(void);

/**
 * Shutdown the Font library
 */
triVoid triFontShutdown(void);

#ifdef TRI_SUPPORT_FT
/**
 * Load a TrueType font.
 *
 * @param filename - Path to the font
 *
 * @param size - Size to set the font to (in points)
 *
 * @param textureSize - Size of the bitmap texture to create (must be power2)
 *
 * @param location - One of ::triTextureLocation
 *
 * @returns A ::triFont struct
 */
triFont* triFontLoad(const triChar *filename, triUInt fontSize, enum triFontSizeType fontSizeType, triUInt textureSize, enum triFontLocation location);
#endif

/**
 * Free the specified font.
 *
 * @param font - A valid ::triFont
 */
triVoid triFontUnload(triFont *font);

/**
 * Load a TRF (triFont) font.
 *
 * @param filename - Path to the font
 *
 * @returns A ::triFont struct
 */
triFont* triFontLoadTRF(const triChar *filename);

/**
 * Save the specified font to a triFont file.
 *
 * @param font - A valid ::triFont
 *
 * @param filename - The file to save to
 */
triVoid triFontSaveTRF(triFont *font, triChar *filename);

/**
 * Make a font mono spaced.
 *
 * @param font - A valid ::triFont or 0 for inbuilt debug font
 *
 * @param width - Width to make all glyphs, 0 to auto decide or < 0 to offset autowidth by that amount
 */
triVoid triFontMakeMono( triFont* font, triS32 width );


/**
 * Activate the specified font.
 *
 * @param font - A valid ::triFont or 0 to activate inbuilt debug font
 */
triVoid triFontActivate(triFont *font);

/**
 * Draw text along the baseline starting at x, y.
 *
 * @param x - X position on screen
 *
 * @param y - Y position on screen
 *
 * @param color - Text color
 *
 * @param text - Text to draw
 *
 * @returns The total width of the largest line in the text drawn.
 */
triSInt triFontPrint(triFloat x, triFloat y, triUInt color, const triChar *text);

/**
 * Draw text along the baseline starting at x, y (with formatting).
 *
 * @param x - X position on screen
 *
 * @param y - Y position on screen
 *
 * @param color - Text color
 *
 * @param text - Text to draw
 *
 * @returns The total width of the largest line in the text drawn.
 */
triSInt triFontPrintf(triFloat x, triFloat y, triUInt color, const triChar *text, ...);

/**
 * Measure a length of text if it were to be drawn
 *
 * @param text - Text to measure
 *
 * @returns The total width of the largest line in the text.
 */
triSInt triFontMeasureText(const triChar *text);

/** @} */

#endif // __TRIFONT_H__
