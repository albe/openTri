#ifndef __RLE_H__
#define __RLE_H__

//#define assembler_decode__

void decodeRLE( void* src, int srcLen, void* dst, int dstLen, int bits );

// returns RLE encoded size of data, linesz gives number of pixels after which to stop any runs
// dstLen should be srcLen + srcLen/4 to make sure all of src can be encoded.
long encodeRLE( void* src, int srcLen, int linesz, void* dst, int dstLen, int bits );

#endif
