#ifndef __TGA_H__
#define __TGA_H__

// ImageType Codes
#define TGA_TYPE_MAPPED		1
#define TGA_TYPE_COLOR		2
#define TGA_TYPE_GRAY		3
#define TGA_TYPE_MAPPED_RLE	9
#define TGA_TYPE_COLOR_RLE	10
#define TGA_TYPE_GRAY_RLE	11


/* Image descriptor:
     3-0: attribute (alpha) bpp
     4:   left-to-right ordering
     5:   top-to-bottom ordering
     7-6: zero
*/
// Image Description Bitmasks
#define TGA_DESC_ABITS		0x0f			// Alpha Bits
#define TGA_DESC_HORIZONTAL	0x10			// Left-Right Ordering: 0 = left to right, 1 = right to left
#define TGA_DESC_VERTICAL	0x20			// Top-Bottom Ordering: 0 = bottom to top, 1 = top to bottom
#define uchar unsigned char

typedef struct {
    uchar	ImageIDSize;
    uchar	ColorMapType;
    uchar	ImageTypeCode;                          // Image Type (normal/paletted/grayscale/rle)
    uchar	ColorMapOrigin[2];
    uchar	ColorMapLength[2];                      // Palette Size
    uchar	ColorMapESize;							// Size in bits of one Palette entry
    uchar	OriginX[2];
    uchar	OriginY[2];
    uchar	Width[2];                               // Width of Image
    uchar	Height[2];                              // Height of Image
    uchar	Depth;                                  // Bits per Pixel of Image
    uchar	ImageDescrip;
} TGAFILEHEADER;

#endif
