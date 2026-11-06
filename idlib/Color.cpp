/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").  

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "precompiled.h"
#pragma hdrstop

#include "Color.h"

/*
===============================================================================

	Colors

===============================================================================
*/

idVec4	colorBlack	= idVec4( 0.00f, 0.00f, 0.00f, 1.00f );
idVec4	colorWhite	= idVec4( 1.00f, 1.00f, 1.00f, 1.00f );
idVec4	colorRed	= idVec4( 1.00f, 0.00f, 0.00f, 1.00f );
idVec4	colorGreen	= idVec4( 0.00f, 1.00f, 0.00f, 1.00f );
idVec4	colorBlue	= idVec4( 0.00f, 0.00f, 1.00f, 1.00f );
idVec4	colorYellow	= idVec4( 1.00f, 1.00f, 0.00f, 1.00f );
idVec4	colorMagenta= idVec4( 1.00f, 0.00f, 1.00f, 1.00f );
idVec4	colorCyan	= idVec4( 0.00f, 1.00f, 1.00f, 1.00f );
idVec4	colorOrange	= idVec4( 1.00f, 0.50f, 0.00f, 1.00f );
idVec4	colorPurple	= idVec4( 0.60f, 0.00f, 0.60f, 1.00f );
idVec4	colorPink	= idVec4( 0.73f, 0.40f, 0.48f, 1.00f );
idVec4	colorBrown	= idVec4( 0.40f, 0.35f, 0.08f, 1.00f );
idVec4	colorLtGrey	= idVec4( 0.75f, 0.75f, 0.75f, 1.00f );
idVec4	colorMdGrey	= idVec4( 0.50f, 0.50f, 0.50f, 1.00f );
idVec4	colorDkGrey	= idVec4( 0.25f, 0.25f, 0.25f, 1.00f );

/*
================
PackColor
================
*/
dword PackColor( const idVec4 &color ) {
	byte dx = idMath::Ftob( color.x * 255.0f );
	byte dy = idMath::Ftob( color.y * 255.0f );
	byte dz = idMath::Ftob( color.z * 255.0f );
	byte dw = idMath::Ftob( color.w * 255.0f );
	return ( dx << 0 ) | ( dy << 8 ) | ( dz << 16 ) | ( dw << 24 );
}

/*
================
UnpackColor
================
*/
void UnpackColor( const dword color, idVec4 &unpackedColor ) {
	unpackedColor.Set( ( ( color >> 0 ) & 255 ) * ( 1.0f / 255.0f ),
						( ( color >> 8 ) & 255 ) * ( 1.0f / 255.0f ), 
						( ( color >> 16 ) & 255 ) * ( 1.0f / 255.0f ),
						( ( color >> 24 ) & 255 ) * ( 1.0f / 255.0f ) );
}

/*
================
PackColor
================
*/
dword PackColor( const idVec3 &color ) {
	byte dx = idMath::Ftob( color.x * 255.0f );
	byte dy = idMath::Ftob( color.y * 255.0f );
	byte dz = idMath::Ftob( color.z * 255.0f );
	return ( dx << 0 ) | ( dy << 8 ) | ( dz << 16 );
}

/*
================
UnpackColor
================
*/
void UnpackColor( const dword color, idVec3 &unpackedColor ) {
	unpackedColor.Set( ( ( color >> 0 ) & 255 ) * ( 1.0f / 255.0f ),
						( ( color >> 8 ) & 255 ) * ( 1.0f / 255.0f ), 
						( ( color >> 16 ) & 255 ) * ( 1.0f / 255.0f ) );
}

/*
================
ColorBytes3
================
*/
unsigned ColorBytes3( float r, float g, float b ) {
	unsigned i;

	( (byte *)&i )[0] = r * 255;
	( (byte *)&i )[1] = g * 255;
	( (byte *)&i )[2] = b * 255;

	return i;
}

/*
================
ColorBytes4
================
*/
unsigned ColorBytes4( float r, float g, float b, float a ) {
	unsigned i;

	( (byte *)&i )[0] = r * 255;
	( (byte *)&i )[1] = g * 255;
	( (byte *)&i )[2] = b * 255;
	( (byte *)&i )[3] = a * 255;

	return i;
}

/*
================
NormalizeColor
================
*/
float NormalizeColor( const idVec3 &in, idVec3 &out ) {
	float max;

	max = in[0];
	if ( in[1] > max ) {
		max = in[1];
	}
	if ( in[2] > max ) {
		max = in[2];
	}

	if ( !max ) {
		out.Zero();
	} else {
		out[0] = in[0] / max;
		out[1] = in[1] / max;
		out[2] = in[2] / max;
	}
	return max;
}