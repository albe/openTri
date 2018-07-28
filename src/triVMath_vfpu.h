/*
 * triVMath_vfpu.h: Header for Vector maths using PSP's VFPU
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


#ifndef __TRIVMATH_VFPU_H__
#define __TRIVMATH_VFPU_H__


#include "triTypes.h"


/** @addtogroup triVec2
 *  @{
 */


triVec2f* triVec2Set( triVec2f* a, const triFloat x, const triFloat y );
triVec2i* triVec2Ceil( triVec2i* a, const triVec2f* b );
triVec2i* triVec2Trunc( triVec2i* a, const triVec2f* b );
triVec2i* triVec2Round( triVec2i* a, const triVec2f* b );
triVec2i* triVec2Floor( triVec2i* a, const triVec2f* b );
triVec2f* triVec2i2f( triVec2f* a, const triVec2i* b );
triVec2f* triVec2Add( triVec2f* a, const triVec2f* b, const triVec2f* c );
triVec2f* triVec2Sub( triVec2f* a, const triVec2f* b, const triVec2f* c );
triVec2f* triVec2Mul( triVec2f* a, const triVec2f* b, const triVec2f* c );
triVec2f* triVec2Div( triVec2f* a, const triVec2f* b, const triVec2f* c );
triVec2f* triVec2Neg( triVec2f* a, const triVec2f* b );
triVec2f* triVec2Abs( triVec2f* a, const triVec2f* b );
triVec2f* triVec2Rndn( triVec2f* a );
triVec2f* triVec2Rnd( triVec2f* a );
triVec2f* triVec2Rnd2( triVec2f* a );
triVec2f* triVec2Clamp( triVec2f* a, const triVec2f* b, triFloat min, triFloat max );
triVec2f* triVec2Min( triVec2f* a, const triVec2f* b, const triVec2f* c );
triVec2f* triVec2Max( triVec2f* a, const triVec2f* b, const triVec2f* c );
triFloat triVec2Sum( const triVec2f* a );
triFloat triVec2Avg( const triVec2f* a );
triVec2f* triVec2Sgn( triVec2f* a, const triVec2f* b );
triVec2f* triVec2Normalize( triVec2f* a, const triVec2f* b );
triFloat triVec2Length( const triVec2f* a );
triFloat triVec2SquareLength( const triVec2f* a );
triFloat triVec2Dist( const triVec2f* a, const triVec2f* b );
triFloat triVec2SquareDist( const triVec2f* a, const triVec2f* b );
triVec2f* triVec2Lerp( triVec2f* a, const triVec2f* b, const triVec2f* c, triFloat t );
triVec2f* triVec2Scale( triVec2f* a, const triVec2f* b, triFloat t );
triFloat triVec2Dot( const triVec2f* a, const triVec2f* b );
triVec2f* triVec2Reflect( triVec2f* a, const triVec2f* b, const triVec2f* c );
triVec2f* triVec2Refract( triVec2f* a, const triVec2f* b, const triVec2f* c, const triFloat eta );

/** @} */  // End triVec2


/** @addtogroup triVec3
 *  @{
 */

triVec3f* triVec3Set( triVec3f* a, const triFloat x, const triFloat y, const triFloat z );
triVec3i* triVec3Ceil( triVec3i* a, const triVec3f* b );
triVec3i* triVec3Trunc( triVec3i* a, const triVec3f* b );
triVec3i* triVec3Round( triVec3i* a, const triVec3f* b );
triVec3i* triVec3Floor( triVec3i* a, const triVec3f* b );
triVec3f* triVec3i2f( triVec3f* a, const triVec3i* b );
triVec3f* triVec3Add( triVec3f* a, const triVec3f* b, const triVec3f* c );
triVec3f* triVec3Sub( triVec3f* a, const triVec3f* b, const triVec3f* c );
triVec3f* triVec3Mul( triVec3f* a, const triVec3f* b, const triVec3f* c );
triVec3f* triVec3Div( triVec3f* a, const triVec3f* b, const triVec3f* c );
triVec3f* triVec3Neg( triVec3f* a, const triVec3f* b );
triVec3f* triVec3Abs( triVec3f* a, const triVec3f* b );
triVec3f* triVec3Rndn( triVec3f* a );
triVec3f* triVec3Rnd( triVec3f* a );
triVec3f* triVec3Rnd2( triVec3f* a );
triVec3f* triVec3Clamp( triVec3f* a, const triVec3f* b, triFloat min, triFloat max );
triVec3f* triVec3Min( triVec3f* a, const triVec3f* b, const triVec3f* c );
triVec3f* triVec3Max( triVec3f* a, const triVec3f* b, const triVec3f* c );
triFloat triVec3Sum( const triVec3f* a );
triFloat triVec3Avg( const triVec3f* a );
triVec3f* triVec3Sgn( triVec3f* a, const triVec3f* b );
triVec3f* triVec3Normalize( triVec3f* a, const triVec3f* b );
triFloat triVec3Length( const triVec3f* a );
triFloat triVec3SquareLength( const triVec3f* a );
triFloat triVec3Dist( const triVec3f* a, const triVec3f* b );
triFloat triVec3SquareDist( const triVec3f* a, const triVec3f* b );
triVec3f* triVec3Lerp( triVec3f* a, const triVec3f* b, const triVec3f* c, triFloat t );
triVec3f* triVec3Scale( triVec3f* a, const triVec3f* b, triFloat t );
triFloat triVec3Dot( const triVec3f* a, const triVec3f* b );
triVec3f* triVec3Cross( triVec3f* a, const triVec3f* b, const triVec3f* c );
triVec3f* triVec3Reflect( triVec3f* a, const triVec3f* b, const triVec3f* c );
triVec3f* triVec3Refract( triVec3f* a, const triVec3f* b, const triVec3f* c, const triFloat eta );


/** @} */  // End triVec3


/** @addtogroup triVec4
 *  @{
 */


triVec4f* triVec4Set( triVec4f* a, const triFloat x, const triFloat y, const triFloat z, const triFloat w );
triVec4f* triVec4Set3( triVec4f* a, const triFloat x, const triFloat y, const triFloat z );
triVec4i* triVec4Ceil( triVec4i* a, const triVec4f* b );
triVec4i* triVec4Trunc( triVec4i* a, const triVec4f* b );
triVec4i* triVec4Round( triVec4i* a, const triVec4f* b );
triVec4i* triVec4Floor( triVec4i* a, const triVec4f* b );
triVec4f* triVec4i2f( triVec4f* a, const triVec4i* b );


/** Random normal vector.
  * Creates a random unit vector lying on the 3D unit sphere.
  * The fourth component lies in range [-1.0, 1.0[
  * @param a - Pointer to destination vector
  * @return Pointer to a
  */
triVec4f* triVec4Rndn3( triVec4f* a );

/** Random normal vector.
  * Creates a random unit vector lying on the unit sphere.
  * @param a - Pointer to destination vector
  * @return Pointer to a
  */
triVec4f* triVec4Rndn( triVec4f* a );

/** Random vector.
  * Creates random components in range [0.0, 1.0[
  * @param a - Pointer to destination vector
  * @return Pointer to a
  */
triVec4f* triVec4Rnd( triVec4f* a );

/** Random vector.
  * Creates random components in range [-1.0, 1.0[
  * @param a - Pointer to destination vector
  * @return Pointer to a
  */
triVec4f* triVec4Rnd2( triVec4f* a );

/** Add Vectors.
  * a = b.x+c.x | b.y+c.y | b.z+c.z | b.w+c.w
  * @param a - Pointer to destination vector
  * @param b - Pointer to first source vector
  * @param c - Pointer to second source vector
  * @return Pointer to a
  */
triVec4f* triVec4Add( triVec4f* a, const triVec4f* b, const triVec4f* c );

/** Add Vectors.
  * a = b.x+c.x | b.y+c.y | b.z+c.z | b.w
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to destination vector
  * @param b - Pointer to first source vector
  * @param c - Pointer to second source vector
  * @return Pointer to a
  */
triVec4f* triVec4Add3( triVec4f* a, const triVec4f* b, const triVec4f* c );

/** Subtract Vectors.
  * a = b.x-c.x | b.y-c.y | b.z-c.z | b.w-c.w
  * @param a - Pointer to destination vector
  * @param b - Pointer to first source vector
  * @param c - Pointer to second source vector
  * @return Pointer to a
  */
triVec4f* triVec4Sub( triVec4f* a, const triVec4f* b, const triVec4f* c );

/** Subtract Vectors.
  * a = b.x-c.x | b.y-c.y | b.z-c.z | b.w
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to destination vector
  * @param b - Pointer to first source vector
  * @param c - Pointer to second source vector
  * @return Pointer to a
  */
triVec4f* triVec4Sub3( triVec4f* a, const triVec4f* b, const triVec4f* c );

/** Multiply Vectors.
  * a = b.x*c.x | b.y*c.y | b.z*c.z | b.w*c.w
  * @param a - Pointer to destination vector
  * @param b - Pointer to first source vector
  * @param c - Pointer to second source vector
  * @return Pointer to a
  */
triVec4f* triVec4Mul( triVec4f* a, const triVec4f* b, const triVec4f* c );

/** Multiply Vectors.
  * a = b.x*c.x | b.y*c.y | b.z*c.z | b.w
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to destination vector
  * @param b - Pointer to first source vector
  * @param c - Pointer to second source vector
  * @return Pointer to a
  */
triVec4f* triVec4Mul3( triVec4f* a, const triVec4f* b, const triVec4f* c );

/** Divide Vectors.
  * a = b.x/c.x | b.y/c.y | b.z/c.z | b.w/c.w
  * @param a - Pointer to destination vector
  * @param b - Pointer to first source vector
  * @param c - Pointer to second source vector
  * @return Pointer to a
  */
triVec4f* triVec4Div( triVec4f* a, const triVec4f* b, const triVec4f* c );

/** Divide Vectors.
  * a = b.x/c.x | b.y/c.y | b.z/c.z | b.w
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to destination vector
  * @param b - Pointer to first source vector
  * @param c - Pointer to second source vector
  * @return Pointer to a
  */
triVec4f* triVec4Div3( triVec4f* a, const triVec4f* b, const triVec4f* c );

/** Negate Vector.
  * a = -b
  * @param a - Pointer to destination vector
  * @param b - Pointer to source vector
  * @return Pointer to a
  */
triVec4f* triVec4Neg( triVec4f* a, const triVec4f* b );

/** Negate Vector.
  * a = -b
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to destination vector
  * @param b - Pointer to source vector
  * @return Pointer to a
  */
triVec4f* triVec4Neg3( triVec4f* a, const triVec4f* b );

/** Absolute of Vector.
  * a = abs(b.x) | abs(b.y) | abs(b.z) | abs(b.w)
  * @param a - Pointer to destination vector
  * @param b - Pointer to source vector
  * @return Pointer to a
  */
triVec4f* triVec4Abs( triVec4f* a, const triVec4f* b );

/** Clamp Vector.
  * a = clamp(b.x,min,max) | clamp(b.y,min,max) | clamp(b.z,min,max) | clamp(b.w,min,max)
  * @param a - Pointer to destination vector
  * @param b - Pointer to source vector
  * @param min - Minimum to clamp to
  * @param max - Maximum to clamp to
  * @return Pointer to a
  */
triVec4f* triVec4Clamp( triVec4f* a, const triVec4f* b, triFloat min, triFloat max );

/** Clamp Vector.
  * a = clamp(b.x,min,max) | clamp(b.y,min,max) | clamp(b.z,min,max) | b.w
  * @param a - Pointer to destination vector
  * @param b - Pointer to source vector
  * @param min - Minimum to clamp to
  * @param max - Maximum to clamp to
  * @return Pointer to a
  */
triVec4f* triVec4Clamp3( triVec4f* a, const triVec4f* b, triFloat min, triFloat max );

/** Minimum of Vectors.
  * a = min(b.x,c.x) | min(b.y,c.y) | min(b.z,c.z) | min(b.w,c.w)
  * @param a - Pointer to destination vector
  * @param b - Pointer to first source vector
  * @param c - Pointer to second source vector
  * @return Pointer to a
  */
triVec4f* triVec4Min( triVec4f* a, const triVec4f* b, const triVec4f* c );

/** Maximum of Vectors.
  * a = max(b.x,c.x) | max(b.y,c.y) | max(b.z,c.z) | max(b.w,c.w)
  * @param a - Pointer to destination vector
  * @param b - Pointer to first source vector
  * @param c - Pointer to second source vector
  * @return Pointer to a
  */
triVec4f* triVec4Max( triVec4f* a, const triVec4f* b, const triVec4f* c );

/** Sum of components.
  * return (a.x + a.y + a.z + a.w)
  * @param a - Pointer to source vector
  * @return Sum
  */
triFloat triVec4Sum( const triVec4f* a );

/** Sum of components.
  * return (a.x + a.y + a.z)
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to source vector
  * @return Sum
  */
triFloat triVec4Sum3( const triVec4f* a );

/** Average of components.
  * return (a.x + a.y + a.z + a.w)/4
  * @param a - Pointer to source vector
  * @return Average
  */
triFloat triVec4Avg( const triVec4f* a );

/** Average of components.
  * return (a.x + a.y + a.z)/3
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to source vector
  * @return Average
  */
triFloat triVec4Avg3( const triVec4f* a );

/** Vector sign.
  * a = sgn(b.x) | sgn(b.y) | sgn(b.z) | sgn(b.w)
  * @param a - Pointer to destination vector
  * @param b - Pointer to source vector
  * @return Pointer to a
  */
triVec4f* triVec4Sgn( triVec4f* a, const triVec4f* b );

/** Normalize Vector.
  * a = b / sqrt(b.b)
  * @param a - Pointer to destination vector
  * @param b - Pointer to source vector
  * @return Pointer to a
  */
triVec4f* triVec4Normalize( triVec4f* a, const triVec4f* b );

/** Normalize Vector.
  * a = b / sqrt(b.b)
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to destination vector
  * @param b - Pointer to source vector
  * @return Pointer to a
  */
triVec4f* triVec4Normalize3( triVec4f* a, const triVec4f* b );

/** Length of Vector.
  * return sqrt( a.a )
  * @param a - Pointer to source vector
  * @return Lenght
  */
triFloat triVec4Length( const triVec4f* a );

/** Length of Vector.
  * return sqrt( a.a )
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to source vector
  * @return Lenght
  */
triFloat triVec4Length3( const triVec4f* a );

/** Square length of Vector.
  * return a.a
  * @param a - Pointer to source vector
  * @return Square lenght
  */
triFloat triVec4SquareLength( const triVec4f* a );

/** Square length of Vector.
  * return a.a
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to source vector
  * @return Square lenght
  */
triFloat triVec4SquareLength3( const triVec4f* a );

/** Distance between Vectors.
  * return sqrt( (b-a).(b-a) )
  * @param a - Pointer to first source vector
  * @param b - Pointer to second source vector
  * @return Distance
  */
triFloat triVec4Dist( const triVec4f* a, const triVec4f* b );

/** Distance between Vectors.
  * return sqrt( (b-a).(b-a) )
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to first source vector
  * @param b - Pointer to second source vector
  * @return Distance
  */
triFloat triVec4Dist3( const triVec4f* a, const triVec4f* b );

/** Square distance between Vectors.
  * return (b-a).(b-a)
  * @param a - Pointer to first source vector
  * @param b - Pointer to second source vector
  * @return Square distance
  */
triFloat triVec4SquareDist( const triVec4f* a, const triVec4f* b );

/** Square distance between Vectors.
  * return (b-a).(b-a)
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to first source vector
  * @param b - Pointer to second source vector
  * @return Square distance
  */
triFloat triVec4SquareDist3( const triVec4f* a, const triVec4f* b );

/** Linear interpolate between Vectors.
  * a = b * (1 - t) + c * t
  * @param a - Pointer to destination vector
  * @param b - Pointer to first source vector
  * @param c - Pointer to second source vector
  * @param t - Scalar to interpolate with
  * @return Pointer to a
  */
triVec4f* triVec4Lerp( triVec4f* a, const triVec4f* b, const triVec4f* c, triFloat t );

/** Linear interpolate between Vectors.
  * a = b * (1 - t) + c * t
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to destination vector
  * @param b - Pointer to first source vector
  * @param c - Pointer to second source vector
  * @param t - Scalar to interpolate with
  * @return Pointer to a
  */
triVec4f* triVec4Lerp3( triVec4f* a, const triVec4f* b, const triVec4f* c, triFloat t );

/** Scale Vector.
  * a = b * t
  * @param a - Pointer to destination vector
  * @param b - Pointer to source vector
  * @param t - Scalar to scale b with
  * @return Pointer to a
  */
triVec4f* triVec4Scale( triVec4f* a, const triVec4f* b, triFloat t );

/** Scale Vector.
  * a = b * t
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to destination vector
  * @param b - Pointer to source vector
  * @param t - Scalar to scale b with
  * @return Pointer to a
  */
triVec4f* triVec4Scale3( triVec4f* a, const triVec4f* b, triFloat t );

/** Calculate Vector dotproduct.
  * return a.b
  * @param a - Pointer to first source vector
  * @param b - Pointer to second source vector
  * @return Dotproduct of a and b
  */
triFloat triVec4Dot( const triVec4f* a, const triVec4f* b );

/** Calculate Vector dotproduct.
  * return a.b
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to first source vector
  * @param b - Pointer to second source vector
  * @return Dotproduct of a and b
  */
triFloat triVec4Dot3( const triVec4f* a, const triVec4f* b );

/** Calculate Vector crossproduct.
  * a = b x c
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to destination vector
  * @param b - Pointer to first source vector
  * @param c - Pointer to second source vector
  * @return Pointer to a
  */
triVec4f* triVec4Cross( triVec4f* a, const triVec4f* b, const triVec4f* c );

/** Reflect Vector.
  * Reflect b on the normale c.
  * a = b - 2*(b.c)*c
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to destination vector
  * @param b - Pointer to incoming vector
  * @param c - Pointer to surface normale
  * @return Pointer to a
  */
triVec4f* triVec4Reflect( triVec4f* a, const triVec4f* b, const triVec4f* c );

/** Refract Vector.
  * Refract b on the normale c as in Snell's law - eta = n1/n2.
  * <br>Vectors are treated as 3D vectors.
  * @param a - Pointer to destination vector
  * @param b - Pointer to incoming vector
  * @param c - Pointer to surface normale
  * @param eta - Relative refraction index n1/n2
  * @return Pointer to a
  */
triVec4f* triVec4Refract( triVec4f* a, const triVec4f* b, const triVec4f* c, const triFloat eta );

/** @} */  // End triVec3


/** @addtogroup triMat
 *  @{
 */

/**
  * Make a matrix identity.
  * Matrix will contain the following after call:<br>
  * | 1.0  0.0  0.0  0.0 |<br>
  * | 0.0  1.0  0.0  0.0 |<br>
  * | 0.0  0.0  1.0  0.0 |<br>
  * | 0.0  0.0  0.0  1.0 |<br>
  * @param a - Pointer to destination matrix
  * @return Pointer to a
  */
triMat4f* triMat4Identity( triMat4f* a );

/** Make a matrix zero.
  * Matrix will contain the following after call:<br>
  * | 0.0  0.0  0.0  0.0 |<br>
  * | 0.0  0.0  0.0  0.0 |<br>
  * | 0.0  0.0  0.0  0.0 |<br>
  * | 0.0  0.0  0.0  0.0 |<br>
  * @param a - Pointer to destination matrix
  * @return Pointer to a
  */
triMat4f* triMat4Zero( triMat4f* a );

/** Copy a matrix.
  * @param a - Pointer to destination matrix
  * @param b - Pointer to matrix to be copied
  * @return Pointer to a
  */
triMat4f* triMat4Copy( triMat4f* a, const triMat4f* b );

/** Matrix multiply (a = b*c).
  * @param a - Pointer to destination matrix
  * @param b - Pointer to first input matrix
  * @param c - Pointer to second input matrix
  * @return Pointer to a
  */
triMat4f* triMat4Mul( triMat4f* a, const triMat4f* b, const triMat4f* c );

/** Invert a matrix.
  * Full Inverse. For orthogonal matrices, triMat4Trans does the same.
  * @param a - Pointer to destination matrix
  * @param b - Pointer to matrix to be inverted
  * @return Pointer to a
  */
triMat4f* triMat4Inv( triMat4f* a, const triMat4f* b );

/** Transpose a matrix.
  * @param a - Pointer to destination matrix
  * @param b - Pointer to matrix to be transposed
  * @return Pointer to a
  */
triMat4f* triMat4Trans( triMat4f* a, const triMat4f* b );

/** Calculate matrix determinant.
  * @param a - Pointer to source matrix
  * @return Determinant of matrix a
  */
triFloat triMat4Det( const triMat4f* a );

/** Calculate matrix trace.
  * @param a - Pointer to source matrix
  * @return Trace of matrix a
  */
triFloat triMat4Trace( const triMat4f* a );

/** 4x4 Matrix-Vector multiply (a = b*c).
  * @param a - Pointer to dest vector
  * @param b - Pointer to source matrix
  * @param c - Pointer to source vector
  * @return Pointer to a
  */
triVec4f* triMat4Apply( triVec4f* a, const triMat4f* b, const triVec4f* c );

/** 3x3 Matrix-Vector multiply (a = b*c).
  * Vectors are treated as 3D Vectors and the matrix as 3D matrix.
  * This is useful if you only want to rotate a vector by the 4x4 Matrix (no translation).
  * @param a - Pointer to dest vector
  * @param b - Pointer to source matrix
  * @param c - Pointer to source vector
  * @return Pointer to a
  */
triVec4f* triMat4Apply3( triVec4f* a, const triMat4f* b, const triVec4f* c );

/** @} */  // End triMat




/** @defgroup triQuat Quaternions
 *  @{
 */

/** Make unit quaternion (0,0,0,1).
  * @param a - Pointer to dest quaternion
  * @return Pointer to a
  */
triQuat* triQuatUnit(triQuat* a);

/** Copy quaternion.
  * @param a - Pointer to dest quaternion
  * @param b - Pointer to source quaternion
  * @return Pointer to a
  */
triQuat* triQuatCopy(triQuat* a, const triQuat* b);

/** Convert quaternion to matrix.
  * @param a - Pointer to dest matrix
  * @param b - Pointer to source quaternion
  * @return Pointer to a
  */
triMat4f* triQuatToMatrix(triMat4f* a, const triQuat* b);

/** Apply quaternion to vector.
  * @param a - Pointer to dest vector
  * @param b - Pointer to source quaternion
  * @param c - Pointer to source vector
  * @return Pointer to a
  */
triVec4f* triQuatApply(triVec4f* a, const triQuat* b, const triVec4f* c);

/** Add quaternions.
  * @param a - Pointer to dest quaternion
  * @param b - Pointer to first source quaternion
  * @param c - Pointer to second source quaternion
  * @return Pointer to a
  */
triQuat* triQuatAdd(triQuat* a, const triQuat* b, const triQuat* c);

/** Subtract quaternions.
  * @param a - Pointer to dest quaternion
  * @param b - Pointer to first source quaternion
  * @param c - Pointer to second source quaternion
  * @return Pointer to a
  */
triQuat* triQuatSub(triQuat* a, const triQuat* b, const triQuat* c);

/** Multiply quaternions.
  * @param a - Pointer to dest quaternion
  * @param b - Pointer to first source quaternion
  * @param c - Pointer to second source quaternion
  * @return Pointer to a
  */
triQuat* triQuatMul(triQuat* a, const triQuat* b, const triQuat* c);

/** Quaternion innerproduct (dotproduct).
  * @param a - Pointer to first source quaternion
  * @param b - Pointer to second source quaternion
  * @return Dotproduct of quaternions
  */
triFloat triQuatInnerProduct(const triQuat* a, const triQuat* b);

/** Normal (linear) interpolation of quaternions.
  * @param a - Pointer to dest quaternion
  * @param b - Pointer to first source quaternion
  * @param c - Pointer to second source quaternion
  * @param t - Interpolation step
  * @return Pointer to a
  */
triQuat* triQuatNLerp(triQuat* a, const triQuat* b, const triQuat* c, triFloat t);

/** Spherical interpolation of quaternions.
  * @param a - Pointer to dest quaternion
  * @param b - Pointer to first source quaternion
  * @param c - Pointer to second source quaternion
  * @param t - Interpolation step
  * @return Pointer to a
  */
triQuat* triQuatSLerp(triQuat* a, const triQuat* b, const triQuat* c, triFloat t);

/** Cubical interpolation of quaternions.
  * return SLerp( SLerp( b, c, t ), SLerp( d, e, t ), 2*t*(1-t) )
  * @param a - Pointer to dest quaternion
  * @param b - Pointer to first source quaternion
  * @param c - Pointer to second source quaternion
  * @param d - Pointer to third source quaternion
  * @param e - Pointer to fourth source quaternion
  * @param t - Interpolation step
  * @return Pointer to a
  */
triQuat* triQuatSquad(triQuat* a, const triQuat* b, const triQuat* c,
								  const triQuat* d, const triQuat* e, triFloat t);

/** Normalize quaternion.
  * @param a - Pointer to dest quaternion
  * @param b - Pointer to source quaternion
  * @return Pointer to a
  */
triQuat* triQuatNormalize(triQuat* a, const triQuat* b);

/** Conjugate quaternion (-x,-y,-z,-w).
  * @param a - Pointer to dest quaternion
  * @param b - Pointer to source quaternion
  * @return Pointer to a
  */
triQuat* triQuatConj(triQuat* a, const triQuat* b);

/** Inverse quaternion (normalize(-x,-y,-z,w)).
  * @param a - Pointer to dest quaternion
  * @param b - Pointer to source quaternion
  * @return Pointer to a
  */
triQuat* triQuatInverse(triQuat* a, const triQuat* b);

/** Make quaternion from axis rotation.
  * @param a - Pointer to dest quaternion
  * @param angle - angle in radians to rotate about
  * @param b - Pointer to vector describing the axis to rotate about
  * @return Pointer to a
  */
triQuat* triQuatFromRotate(triQuat* a, triFloat angle, const triVec4f* b);

/** @} */  // End triQuat



triColor4f* triColor4Set( triColor4f* c, const triFloat r, const triFloat g, const triFloat b, const triFloat a );
triColor4f* triColor4Set3( triColor4f* c, const triFloat r, const triFloat g, const triFloat b );
triColor4f* triColor4From4i( triColor4f* a, triColor4i* b );
triColor4f* triColor4From8888( triColor4f* a, triColor8888* b );
triU32 triColor4f2RGBA8888( triColor4f* a );


#endif // __TRIVMATH_VFPU_H__
