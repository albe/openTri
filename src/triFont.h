/*
 * triFont.h: Header for bitmap font library
 * This file is part of the "tri Engine".
 *
 * Copyright (C) 2007 tri
 * Copyright (C) 2007 David Perry 'InsertWittyName' <tias_dp@hotmail.com>
 * Copyright (C) 2008, 2010 Alexander Berl 'Raphael' <raphael@fx-world.org>
 *
 * $Id:$
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
	triSInt	fixedWidth; /**<  Width for mono spaced output */
	triSInt fixedHeight; /**< Height for fixed line height */
	triSInt	letterSpacing; /**< Spacing adjustment for all characters */
	triSInt wordSpacing; /**< Spacing adjustment for all whitespaces */
	triFloat justify; /**< Justify adjustment for the current line */
} triFont;

enum triFontSizeType
{
	TRI_FONT_SIZE_PIXELS = 0,
	TRI_FONT_SIZE_POINTS
};

enum triFontAlignMode
{
	TRI_FONT_ALIGN_LEFT = 0x0,
	TRI_FONT_ALIGN_CENTER = 0x1,
	TRI_FONT_ALIGN_RIGHT = 0x2,
	TRI_FONT_ALIGN_JUSTIFY = 0x3,
	TRI_FONT_HALIGN = 0xF,
	TRI_FONT_ALIGN_TOP = 0x00,
	TRI_FONT_ALIGN_MIDDLE = 0x10,
	TRI_FONT_ALIGN_BOTTOM = 0x20,
	TRI_FONT_VALIGN = 0xF0
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
 * @param size - Size to set the font to (in points)
 * @param textureSize - Size of the bitmap texture to create (must be power2)
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
 * @param filename - The file to save to
 */
triVoid triFontSaveTRF(triFont *font, triChar *filename);

/**
 * Set a font mono spaced or not (default).
 *
 * @param font - A valid ::triFont or 0 for inbuilt debug font
 * @param width - Width to make all glyphs, 0 to make proportional width or < 0 to offset autowidth by that amount+1
 */
triVoid triFontSetMono( triFont* font, triS32 width );

/**
 * Set a spacing values.
 *
 * @param font - A valid ::triFont or 0 for inbuilt debug font
 * @param letter - Spacing to apply to all glyphs: < 0 for tighter spacing, > 0 for wider spacing, 0 for normal spacing (default)
 * @param word - Spacing to apply to all whitespace glyphs in addition to letterspacing: < 0 for tighter spacing, > 0 for wider spacing, 0 for normal spacing (default)
 */
triVoid triFontSetSpacing( triFont* font, triS32 letter, triS32 word );

/**
 * Set line height.
 *
 * @param font - A valid ::triFont or 0 for inbuilt debug font
 * @param height - Height of each line in pixels. 0 for default height.
 */
triVoid triFontSetLineheight( triFont* font, triS32 height );

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
 * @param y - Y position on screen
 * @param color - Text color
 * @param text - Text to draw
 *
 * @returns The total width of the largest line in the text drawn.
 */
triSInt triFontPrint(triFloat x, triFloat y, triUInt color, const triChar *text);

/**
 * Draw text along the baseline starting at x, y (with formatting).
 *
 * @param x - X position on screen
 * @param y - Y position on screen
 * @param color - Text color
 * @param text - Text to draw
 *
 * @returns The total width of the largest line in the text drawn.
 */
triSInt triFontPrintf(triFloat x, triFloat y, triUInt color, const triChar *text, ...);


/**
 * Draw text along the baseline aligned within the box starting at x, y.
 *
 * @param x - X position on screen
 * @param y - Y position on screen
 * @param width - The width of the box to align within
 * @param height - The height of the box to align within
 * @param color - Text color
 * @param align - The align method to apply (see triFontAlignMode)
 * @param text - Text to draw
 *
 * @returns The total width of the largest line in the text drawn.
 */
triSInt triFontPrintAlign(triFloat x, triFloat y, triSInt width, triSInt height, triUInt color, enum triFontAlignMode align, const triChar *text);

/**
 * Draw text along the baseline aligned within the box starting at x, y (with formatting).
 *
 * @param x - X position on screen
 * @param y - Y position on screen
 * @param width - The width of the box to align within
 * @param height - The height of the box to align within
 * @param color - Text color
 * @param align - The align method to apply (see triFontAlignMode)
 * @param text - Text to draw
 *
 * @returns The total width of the largest line in the text drawn.
 */
triSInt triFontPrintAlignf(triFloat x, triFloat y, triSInt width, triSInt height, triUInt color, enum triFontAlignMode align, const triChar *text, ...);

/**
 * Measure height of a text if it were to be drawn
 *
 * @param text - Text to measure
 *
 * @returns The total height of the text.
 */
triSInt triFontMeasureTextHeight(const triChar *text);

/**
 * Measure a length of text if it were to be drawn
 *
 * @param text - Text to measure
 *
 * @returns The total width of the largest line in the text.
 */
triSInt triFontMeasureText(const triChar *text);

/**
 * Measure a length of the current line of text if it were to be drawn
 *
 * @param text - Text to measure
 *
 * @returns The total width of the current line in the text.
 */
triSInt triFontMeasureLine(const triChar *text);

/** @} */

#endif // __TRIFONT_H__
