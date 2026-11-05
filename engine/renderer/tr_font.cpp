/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should comply with these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software LLC at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing to id Software LLC, a ZeniMax Media company, 120 Rockville, Maryland 20850 USA.

===========================================================================
*/

// tr_font.c
//
// Fonts come in two flavours:
//
//  1. prerendered glyph data: fonts/fontImage_<pointsize>.dat plus the
//     fonts/fontImage_<page>_<pointsize>.tga pages. The game ships with
//     these and RE_RegisterFont prefers them when they are present.
//  2. runtime rendering: when the .dat file is missing, the named TrueType
//     file is rasterized with stb_truetype (external/stb/stb_truetype.h)
//     into 256x256 pages. This replaces the old FreeType 2.x code that was
//     never enabled in shipping builds.
//
// With r_saveFontData 1 the runtime path writes the .tga pages and the
// .dat file back out so the results can be used as prerendered data.
//
// The UI scripting scale of 1.0 equals a 48 point font, glyphScale keeps
// that convention for any point size requested.

#include "tr_local.h"
#include "../qcommon/qcommon.h"

// stb_truetype is the only TrueType rasterizer linked into the renderer. The
// implementation lives in this file on purpose: tr_font.cpp is its only user.
#pragma warning( push )
#pragma warning( disable : 4100 4127 4189 4244 4245 4267 4310 4456 4457 4459 4701 4702 4706 4806 4996 )
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#pragma warning( pop )

#define MAX_FONTS 6
static int registeredFontCount = 0;
static fontInfo_t registeredFont[MAX_FONTS];

static int fdOffset;
static byte *fdFile;

int readInt() {
	int i = fdFile[fdOffset] + ( fdFile[fdOffset + 1] << 8 ) + ( fdFile[fdOffset + 2] << 16 ) + ( fdFile[fdOffset + 3] << 24 );
	fdOffset += 4;
	return i;
}

typedef union {
	byte fred[4];
	float ffred;
} poor;

float readFloat() {
	poor me;
#ifdef idppc
	me.fred[0] = fdFile[fdOffset + 3];
	me.fred[1] = fdFile[fdOffset + 2];
	me.fred[2] = fdFile[fdOffset + 1];
	me.fred[3] = fdFile[fdOffset + 0];
#else
	me.fred[0] = fdFile[fdOffset + 0];
	me.fred[1] = fdFile[fdOffset + 1];
	me.fred[2] = fdFile[fdOffset + 2];
	me.fred[3] = fdFile[fdOffset + 3];
#endif
	fdOffset += 4;
	return me.ffred;
}

/*
================
WriteTGA

Uncompressed 32 bit TGA output, used by r_saveFontData to drop the
rendered font pages back into the game filesystem.
================
*/
void WriteTGA( char *filename, byte *data, int width, int height ) {
	byte    *buffer;
	int i, c;

	buffer = (byte *) Mem_ClearedAlloc( width * height * 4 + 18, TAG_IDLIB );
	memset( buffer, 0, 18 );
	buffer[2] = 2;      // uncompressed type
	buffer[12] = width & 255;
	buffer[13] = width >> 8;
	buffer[14] = height & 255;
	buffer[15] = height >> 8;
	buffer[16] = 32;    // pixel size

	// swap rgb to bgr
	c = 18 + width * height * 4;
	for ( i = 18 ; i < c ; i += 4 )
	{
		buffer[i] = data[i - 18 + 2];        // blue
		buffer[i + 1] = data[i - 18 + 1];   // green
		buffer[i + 2] = data[i - 18 + 0];   // red
		buffer[i + 3] = data[i - 18 + 3];   // alpha
	}

	ri.FS_WriteFile( filename, buffer, c );

	Mem_Free( buffer );
}

/*
================
R_FontPageToShader

Turns a full 256x256 glyph coverage page into a shader and clears the
page so it can be filled again. Returns the shader handle and leaves the
page name in name so the caller can store it per glyph.
================
*/
static qhandle_t R_FontPageToShader( byte *out, int imageNumber, int pointSize, char *name, int nameSize ) {
	image_t *image;
	byte *imageBuff;
	int k, left;
	byte max;
	float maxScale;
	qhandle_t h;

	// find the highest coverage value so glyphs use the full alpha range
	max = 0;
	for ( k = 0; k < 256 * 256; k++ ) {
		if ( out[k] > max ) {
			max = out[k];
		}
	}
	maxScale = max > 0 ? 255.0f / max : 0.0f;

	imageBuff = (byte *) Mem_ClearedAlloc( 256 * 256 * 4, TAG_IDLIB );
	left = 0;
	for ( k = 0; k < 256 * 256; k++ ) {
		imageBuff[left++] = 255;
		imageBuff[left++] = 255;
		imageBuff[left++] = 255;
		imageBuff[left++] = (byte)( (float)out[k] * maxScale );
	}

	Com_sprintf( name, nameSize, "fonts/fontImage_%i_%i.tga", imageNumber, pointSize );
	if ( r_saveFontData->integer ) {
		WriteTGA( name, imageBuff, 256, 256 );
	}

	image = R_CreateImage( name, imageBuff, 256, 256, qfalse, qfalse, GL_CLAMP );
	h = RE_RegisterShaderFromImage( name, LIGHTMAP_2D, image, qfalse );

	Mem_Free( imageBuff );
	memset( out, 0, 256 * 256 );

	return h;
}

void RE_RegisterFont( const char *fontName, int pointSize, fontInfo_t *font ) {
	stbtt_fontinfo ttfont;
	void *faceData;
	int i, j, len;
	int advance, lsb, x0, y0, x1, y1;
	int xOut, yOut, rowHeight, width, height;
	int lastStart, imageNumber;
	qhandle_t h;
	byte *out;
	float dpi = 72;
	float glyphScale = 72.0f / dpi;   // change the scale to be relative to 1 based on 72 dpi
	float scale;
	char name[1024];

	if ( pointSize <= 0 ) {
		pointSize = 12;
	}
	// we also need to adjust the scale based on point size relative to 48 points as the ui scaling is based on a 48 point font
	glyphScale *= 48.0f / pointSize;

	// make sure the render thread is stopped
	R_SyncRenderThread();

	if ( registeredFontCount >= MAX_FONTS ) {
		ri.Printf( PRINT_ALL, "RE_RegisterFont: Too many fonts registered already.\n" );
		return;
	}

	Com_sprintf( name, sizeof( name ), "fonts/fontImage_%i.dat", pointSize );
	for ( i = 0; i < registeredFontCount; i++ ) {
		if ( idStr::Icmp( name, registeredFont[i].name ) == 0 ) {
			memcpy( font, &registeredFont[i], sizeof( fontInfo_t ) );
			return;
		}
	}

	len = ri.FS_ReadFile( name, NULL );
	if ( len == sizeof( fontInfo_t ) ) {
		ri.FS_ReadFile( name, &faceData );
		fdOffset = 0;
		fdFile = (byte *) faceData;
		for ( i = 0; i < GLYPHS_PER_FONT; i++ ) {
			font->glyphs[i].height      = readInt();
			font->glyphs[i].top         = readInt();
			font->glyphs[i].bottom      = readInt();
			font->glyphs[i].pitch       = readInt();
			font->glyphs[i].xSkip       = readInt();
			font->glyphs[i].imageWidth  = readInt();
			font->glyphs[i].imageHeight = readInt();
			font->glyphs[i].s           = readFloat();
			font->glyphs[i].t           = readFloat();
			font->glyphs[i].s2          = readFloat();
			font->glyphs[i].t2          = readFloat();
			font->glyphs[i].glyph       = readInt();
			memcpy( font->glyphs[i].shaderName, &fdFile[fdOffset], 32 );
			fdOffset += 32;
		}
		font->glyphScale = readFloat();
		memcpy( font->name, &fdFile[fdOffset], MAX_QPATH );

		idStr::Copynz( font->name, name, sizeof( font->name ) );
		for ( i = GLYPH_START; i < GLYPH_END; i++ ) {
			font->glyphs[i].glyph = RE_RegisterShaderNoMip( font->glyphs[i].shaderName );
		}
		memcpy( &registeredFont[registeredFontCount++], font, sizeof( fontInfo_t ) );
		return;
	}

	// no prerendered data, render the TrueType file at runtime
	len = ri.FS_ReadFile( fontName, &faceData );
	if ( len <= 0 ) {
		ri.Printf( PRINT_ALL, "RE_RegisterFont: Unable to read font file\n" );
		return;
	}

	if ( !stbtt_InitFont( &ttfont, (const unsigned char *)faceData, 0 ) ) {
		ri.Printf( PRINT_ALL, "RE_RegisterFont: stb_truetype failed to parse %s\n", fontName );
		ri.FS_FreeFile( faceData );
		return;
	}

	// at 72 dpi one em in pixels equals the point size, which matches what
	// FreeType did when the shipped .dat files were generated
	scale = stbtt_ScaleForMappingEmToPixels( &ttfont, (float)pointSize );

	// glyphs are packed into 256x256 coverage pages, one shader per page
	out = (byte *) Mem_ClearedAlloc( 256 * 256, TAG_IDLIB );
	memset( out, 0, 256 * 256 );

	xOut = 0;
	yOut = 0;
	rowHeight = 0;
	lastStart = GLYPH_START;
	imageNumber = 0;

	for ( i = GLYPH_START; i <= GLYPH_END; i++ ) {
		glyphInfo_t *glyph = &font->glyphs[i];
		int glyphIndex = stbtt_FindGlyphIndex( &ttfont, i );

		stbtt_GetGlyphHMetrics( &ttfont, glyphIndex, &advance, &lsb );
		stbtt_GetGlyphBitmapBox( &ttfont, glyphIndex, scale, scale, &x0, &y0, &x1, &y1 );

		width = x1 - x0;
		height = y1 - y0;
		if ( width < 0 ) {
			width = 0;
		}
		if ( height < 0 ) {
			height = 0;
		}

		// wrap to a new row when the glyph no longer fits
		if ( width > 0 && xOut + width + 1 >= 256 ) {
			xOut = 0;
			yOut += rowHeight + 1;
			rowHeight = 0;
		}
		if ( height > rowHeight ) {
			rowHeight = height;
		}

		// the page is full, flush it and start a new one
		if ( yOut + rowHeight + 1 >= 256 ) {
			h = R_FontPageToShader( out, imageNumber++, pointSize, name, sizeof( name ) );
			for ( j = lastStart; j < i; j++ ) {
				font->glyphs[j].glyph = h;
				idStr::Copynz( font->glyphs[j].shaderName, name, sizeof( font->glyphs[j].shaderName ) );
			}
			lastStart = i;
			xOut = 0;
			yOut = 0;
			rowHeight = 0;
		}

		memset( glyph, 0, sizeof( glyphInfo_t ) );
		glyph->height = height;
		glyph->top = -y0 + 1;
		glyph->bottom = y1;
		glyph->pitch = width;
		glyph->xSkip = (int)( advance * scale ) + 1;
		glyph->imageWidth = width;
		glyph->imageHeight = height;
		glyph->s = (float)xOut / 256;
		glyph->t = (float)yOut / 256;
		glyph->s2 = glyph->s + (float)width / 256;
		glyph->t2 = glyph->t + (float)height / 256;

		if ( width > 0 && height > 0 ) {
			stbtt_MakeGlyphBitmap( &ttfont, out + yOut * 256 + xOut, width, height, 256, scale, scale, glyphIndex );
			xOut += width + 1;
		}
	}

	// flush the last page
	h = R_FontPageToShader( out, imageNumber++, pointSize, name, sizeof( name ) );
	for ( j = lastStart; j <= GLYPH_END; j++ ) {
		font->glyphs[j].glyph = h;
		idStr::Copynz( font->glyphs[j].shaderName, name, sizeof( font->glyphs[j].shaderName ) );
	}

	// store the font under the name the prerendered data would use so the
	// registered font dedup above works for runtime rendered fonts too
	font->glyphScale = glyphScale;
	Com_sprintf( name, sizeof( name ), "fonts/fontImage_%i.dat", pointSize );
	idStr::Copynz( font->name, name, sizeof( font->name ) );

	memcpy( &registeredFont[registeredFontCount], font, sizeof( fontInfo_t ) );
	registeredFontCount++;

	if ( r_saveFontData->integer ) {
		ri.FS_WriteFile( name, font, sizeof( fontInfo_t ) );
	}

	Mem_Free( out );
	ri.FS_FreeFile( faceData );
}


void R_InitFreeType() {
	registeredFontCount = 0;
}


void R_DoneFreeType() {
	registeredFontCount = 0;
}
