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

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "q_shared.h"

/*
============================================================================

					LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/

int Q_isprint( int c ) {
	if ( c >= 0x20 && c <= 0x7E ) {
		return ( 1 );
	}
	return ( 0 );
}

int Q_islower( int c ) {
	if ( c >= 'a' && c <= 'z' ) {
		return ( 1 );
	}
	return ( 0 );
}

int Q_isupper( int c ) {
	if ( c >= 'A' && c <= 'Z' ) {
		return ( 1 );
	}
	return ( 0 );
}

int Q_isalpha( int c ) {
	if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ) {
		return ( 1 );
	}
	return ( 0 );
}

//----(SA)	added
int Q_isnumeric( int c ) {
	if ( c >= '0' && c <= '9' ) {
		return ( 1 );
	}
	return ( 0 );
}

int Q_isalphanumeric( int c ) {
	if ( Q_isalpha( c ) ||
		 Q_isnumeric( c ) ) {
		return( 1 );
	}
	return ( 0 );
}

int Q_isforfilename( int c ) {
	if ( ( Q_isalphanumeric( c ) || c == '_' ) && c != ' ' ) { // space not allowed in filename
		return( 1 );
	}
	return ( 0 );
}
//----(SA)	end


int Q_PrintStrlen( const char *string ) {
	int len;
	const char  *p;

	if ( !string ) {
		return 0;
	}

	len = 0;
	p = string;
	while ( *p ) {
		if ( Q_IsColorString( p ) ) {
			p += 2;
			continue;
		}
		p++;
		len++;
	}

	return len;
}


char *Q_CleanStr( char *string ) {
	char*   d;
	char*   s;
	int c;

	s = string;
	d = string;
	while ( ( c = *s ) != 0 ) {
		if ( Q_IsColorString( s ) ) {
			s++;
		} else if ( c >= 0x20 && c <= 0x7E )   {
			*d++ = c;
		}
		s++;
	}
	*d = '\0';

	return string;
}


void QDECL Com_sprintf( char *dest, int size, const char *fmt, ... ) {
	int len;
	va_list argptr;
	char bigbuffer[32000];      // big, but small enough to fit in PPC stack

	va_start( argptr,fmt );
	len = vsnprintf( bigbuffer, sizeof( bigbuffer ), fmt, argptr );
	va_end( argptr );
	if ( len >= sizeof( bigbuffer ) ) {
		Com_Error( ERR_FATAL, "Com_sprintf: overflowed bigbuffer" );
	}
	if ( len >= size ) {
		Com_Printf( "Com_sprintf: overflow of %i in %i\n", len, size );
	}
	idStr::Copynz( dest, bigbuffer, size );
}

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday

Ridah, modified this into a circular list, to further prevent stepping on
previous strings
============
*/
char    * QDECL va( char *format, ... ) {
	va_list argptr;
	#define MAX_VA_STRING   32000
	static char temp_buffer[MAX_VA_STRING];
	static char string[MAX_VA_STRING];      // in case va is called by nested functions
	static int index = 0;
	char    *buf;
	int len;


	va_start( argptr, format );
	vsnprintf( temp_buffer, sizeof( temp_buffer ), format, argptr );
	va_end( argptr );

	if ( ( len = strlen( temp_buffer ) ) >= MAX_VA_STRING ) {
		Com_Error( ERR_DROP, "Attempted to overrun string in call to va()\n" );
	}

	if ( len + index >= MAX_VA_STRING - 1 ) {
		index = 0;
	}

	buf = &string[index];
	memcpy( buf, temp_buffer, len + 1 );

	index += len + 1;

	return buf;
}

/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
FIXME: overflow check?
===============
*/
char *Info_ValueForKey( const char *s, const char *key ) {
	char pkey[BIG_INFO_KEY];
	static char value[2][BIG_INFO_VALUE];   // use two buffers so compares
											// work without stomping on each other
	static int valueindex = 0;
	char    *o;

	if ( !s || !key ) {
		return "";
	}

	if ( strlen( s ) >= BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_ValueForKey: oversize infostring" );
	}

	valueindex ^= 1;
	if ( *s == '\\' ) {
		s++;
	}
	while ( 1 )
	{
		o = pkey;
		while ( *s != '\\' )
		{
			if ( !*s ) {
				return "";
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while ( *s != '\\' && *s )
		{
			*o++ = *s++;
		}
		*o = 0;

		if ( !idStr::Icmp( key, pkey ) ) {
			return value[valueindex];
		}

		if ( !*s ) {
			break;
		}
		s++;
	}

	return "";
}


/*
===================
Info_NextPair

Used to itterate through all the key/value pairs in an info string
===================
*/
void Info_NextPair( const char **head, char *key, char *value ) {
	char    *o;
	const char  *s;

	s = *head;

	if ( *s == '\\' ) {
		s++;
	}
	key[0] = 0;
	value[0] = 0;

	o = key;
	while ( *s != '\\' ) {
		if ( !*s ) {
			*o = 0;
			*head = s;
			return;
		}
		*o++ = *s++;
	}
	*o = 0;
	s++;

	o = value;
	while ( *s != '\\' && *s ) {
		*o++ = *s++;
	}
	*o = 0;

	*head = s;
}


/*
===================
Info_RemoveKey
===================
*/
void Info_RemoveKey( char *s, const char *key ) {
	char    *start;
	char pkey[MAX_INFO_KEY];
	char value[MAX_INFO_VALUE];
	char    *o;

	if ( strlen( s ) >= MAX_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_RemoveKey: oversize infostring" );
	}

	if ( strchr( key, '\\' ) ) {
		return;
	}

	while ( 1 )
	{
		start = s;
		if ( *s == '\\' ) {
			s++;
		}
		o = pkey;
		while ( *s != '\\' )
		{
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while ( *s != '\\' && *s )
		{
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;

		if ( !strcmp( key, pkey ) ) {
			strcpy( start, s );  // remove this part
			return;
		}

		if ( !*s ) {
			return;
		}
	}

}

/*
===================
Info_RemoveKey_Big
===================
*/
void Info_RemoveKey_Big( char *s, const char *key ) {
	char    *start;
	char pkey[BIG_INFO_KEY];
	char value[BIG_INFO_VALUE];
	char    *o;

	if ( strlen( s ) >= BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_RemoveKey_Big: oversize infostring" );
	}

	if ( strchr( key, '\\' ) ) {
		return;
	}

	while ( 1 )
	{
		start = s;
		if ( *s == '\\' ) {
			s++;
		}
		o = pkey;
		while ( *s != '\\' )
		{
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while ( *s != '\\' && *s )
		{
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;

		if ( !strcmp( key, pkey ) ) {
			strcpy( start, s );  // remove this part
			return;
		}

		if ( !*s ) {
			return;
		}
	}

}




/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing
==================
*/
qboolean Info_Validate( const char *s ) {
	if ( strchr( s, '\"' ) ) {
		return qfalse;
	}
	if ( strchr( s, ';' ) ) {
		return qfalse;
	}
	return qtrue;
}

/*
==================
Info_SetValueForKey

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey( char *s, const char *key, const char *value ) {
	char newi[MAX_INFO_STRING];

	if ( strlen( s ) >= MAX_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_SetValueForKey: oversize infostring" );
	}

	if ( strchr( key, '\\' ) || strchr( value, '\\' ) ) {
		Com_Printf( "Can't use keys or values with a \\\n" );
		return;
	}

	if ( strchr( key, ';' ) || strchr( value, ';' ) ) {
		Com_Printf( "Can't use keys or values with a semicolon\n" );
		return;
	}

	if ( strchr( key, '\"' ) || strchr( value, '\"' ) ) {
		Com_Printf( "Can't use keys or values with a \"\n" );
		return;
	}

	Info_RemoveKey( s, key );
	if ( !value || !strlen( value ) ) {
		return;
	}

	Com_sprintf( newi, sizeof( newi ), "\\%s\\%s", key, value );

	if ( strlen( newi ) + strlen( s ) > MAX_INFO_STRING ) {
		Com_Printf( "Info string length exceeded\n" );
		return;
	}

	strcat( s, newi );
}

/*
==================
Info_SetValueForKey_Big

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey_Big( char *s, const char *key, const char *value ) {
	char newi[BIG_INFO_STRING];

	if ( strlen( s ) >= BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_SetValueForKey: oversize infostring" );
	}

	if ( strchr( key, '\\' ) || strchr( value, '\\' ) ) {
		Com_Printf( "Can't use keys or values with a \\\n" );
		return;
	}

	if ( strchr( key, ';' ) || strchr( value, ';' ) ) {
		Com_Printf( "Can't use keys or values with a semicolon\n" );
		return;
	}

	if ( strchr( key, '\"' ) || strchr( value, '\"' ) ) {
		Com_Printf( "Can't use keys or values with a \"\n" );
		return;
	}

	Info_RemoveKey_Big( s, key );
	if ( !value || !strlen( value ) ) {
		return;
	}

	Com_sprintf( newi, sizeof( newi ), "\\%s\\%s", key, value );

	if ( strlen( newi ) + strlen( s ) > BIG_INFO_STRING ) {
		Com_Printf( "BIG Info string length exceeded\n" );
		return;
	}

	strcat( s, newi );
}




//====================================================================



/*
============================================================================

	ENGINE HELPERS

============================================================================
*/

/*
=================
SetPlaneSignbits
=================
*/
void SetPlaneSignbits( cplane_t *out ) {
	int bits, j;

	// for fast box on planeside test
	bits = 0;
	for ( j = 0 ; j < 3 ; j++ ) {
		if ( out->normal[j] < 0 ) {
			bits |= 1 << j;
		}
	}
	out->signbits = bits;
}

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
int BoxOnPlaneSide( const idVec3 &emins, const idVec3 &emaxs, cplane_t *p ) {
	float dist1, dist2;
	int sides;

// fast axial cases
	if ( p->type < 3 ) {
		if ( p->dist <= emins[p->type] ) {
			return 1;
		}
		if ( p->dist >= emaxs[p->type] ) {
			return 2;
		}
		return 3;
	}

// general case
	switch ( p->signbits )
	{
	case 0:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		break;
	case 1:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		break;
	case 2:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		break;
	case 3:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		break;
	case 4:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		break;
	case 5:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		break;
	case 6:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		break;
	case 7:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;      // shut up compiler
		break;
	}

	sides = 0;
	if ( dist1 >= p->dist ) {
		sides = 1;
	}
	if ( dist2 < p->dist ) {
		sides |= 2;
	}

	return sides;
}
