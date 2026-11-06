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


/*****************************************************************************
 * name:		l_struct.c
 *
 * desc:		structure reading / writing
 *
 *
 *****************************************************************************/

#ifdef BOTLIB
#include "../precompiled.h"

#include "../../engine/qcommon/q_shared.h"
#include "../game/botlib.h"              //for the include of be_interface.h
#include "l_struct.h"
#include "be_interface.h"
#endif //BOTLIB

#ifdef BSPC
//include files for usage in the BSP Converter
#include "../bspc/qbsp.h"
#include "../bspc/l_log.h"
#include "../bspc/l_mem.h"
#include "l_struct.h"

#define qtrue   true
#define qfalse  false
#endif //BSPC

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
fielddef_t *FindField( fielddef_t *defs, const char *name ) {
	int i;

	for ( i = 0; defs[i].name; i++ )
	{
		if ( !strcmp( defs[i].name, name ) ) {
			return &defs[i];
		}
	} //end for
	return NULL;
} //end of the function FindField
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
qboolean ReadNumber( idParser *source, fielddef_t *fd, void *p ) {
	idToken token;
	int negative = qfalse;
	long int intval, intmin = 0, intmax = 0;
	double floatval;

	if ( !source->ExpectAnyToken( &token ) ) {
		return 0;
	}

	//check for minus sign
	if ( token.type == TT_PUNCTUATION ) {
		if ( fd->type & FT_UNSIGNED ) {
			source->Error( "expected unsigned value, found %s", token.c_str() );
			return 0;
		} //end if
		  //if not a minus sign
		if ( strcmp( token.c_str(), "-" ) ) {
			source->Error( "unexpected punctuation %s", token.c_str() );
			return 0;
		} //end if
		negative = qtrue;
		//read the number
		if ( !source->ExpectAnyToken( &token ) ) {
			return 0;
		}
	} //end if
	  //check if it is a number
	if ( token.type != TT_NUMBER ) {
		source->Error( "expected number, found %s", token.c_str() );
		return 0;
	} //end if
	  //check for a float value
	if ( token.subtype & TT_FLOAT ) {
		if ( ( fd->type & FT_TYPE ) != FT_FLOAT ) {
			source->Error( "unexpected float" );
			return 0;
		} //end if
		floatval = token.GetFloatValue();
		if ( negative ) {
			floatval = -floatval;
		}
		if ( fd->type & FT_BOUNDED ) {
			if ( floatval < fd->floatmin || floatval > fd->floatmax ) {
				source->Error( "float out of range [%f, %f]", fd->floatmin, fd->floatmax );
				return 0;
			} //end if
		} //end if
		*(float *) p = (float) floatval;
		return 1;
	} //end if
	  //
	intval = token.GetIntValue();
	if ( negative ) {
		intval = -intval;
	}
	//check bounds
	if ( ( fd->type & FT_TYPE ) == FT_CHAR ) {
		if ( fd->type & FT_UNSIGNED ) {
			intmin = 0; intmax = 255;
		} else {intmin = -128; intmax = 127;}
	} //end if
	if ( ( fd->type & FT_TYPE ) == FT_INT ) {
		if ( fd->type & FT_UNSIGNED ) {
			intmin = 0; intmax = 65535;
		} else {intmin = -32768; intmax = 32767;}
	} //end else if
	if ( ( fd->type & FT_TYPE ) == FT_CHAR || ( fd->type & FT_TYPE ) == FT_INT ) {
		if ( fd->type & FT_BOUNDED ) {
			intmin = Max( intmin, static_cast<long int>( fd->floatmin ) );
			intmax = Min( intmax, static_cast<long int>( fd->floatmax ) );
		} //end if
		if ( intval < intmin || intval > intmax ) {
			source->Error( "value %d out of range [%d, %d]", intval, intmin, intmax );
			return 0;
		} //end if
	} //end if
	else if ( ( fd->type & FT_TYPE ) == FT_FLOAT ) {
		if ( fd->type & FT_BOUNDED ) {
			if ( intval < fd->floatmin || intval > fd->floatmax ) {
				source->Error( "value %d out of range [%f, %f]", intval, fd->floatmin, fd->floatmax );
				return 0;
			} //end if
		} //end if
	} //end else if
	  //store the value
	if ( ( fd->type & FT_TYPE ) == FT_CHAR ) {
		if ( fd->type & FT_UNSIGNED ) {
			*(unsigned char *) p = (unsigned char) intval;
		} else { *(char *) p = (char) intval;}
	} //end if
	else if ( ( fd->type & FT_TYPE ) == FT_INT ) {
		if ( fd->type & FT_UNSIGNED ) {
			*(unsigned int *) p = (unsigned int) intval;
		} else { *(int *) p = (int) intval;}
	} //end else
	else if ( ( fd->type & FT_TYPE ) == FT_FLOAT ) {
		*(float *) p = (float) intval;
	} //end else
	return 1;
} //end of the function ReadNumber
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
qboolean ReadChar( idParser *source, fielddef_t *fd, void *p ) {
	idToken token;

	if ( !source->ExpectAnyToken( &token ) ) {
		return 0;
	}

	//take literals into account
	if ( token.type == TT_LITERAL ) {
		token.Strip( "'" );
		*(char *) p = token[0];
	} //end if
	else
	{
		source->UnreadToken( &token );
		if ( !ReadNumber( source, fd, p ) ) {
			return 0;
		}
	} //end if
	return 1;
} //end of the function ReadChar
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int ReadString( idParser *source, fielddef_t *fd, void *p ) {
	idToken token;

	if ( !source->ExpectTokenType( TT_STRING, 0, &token ) ) {
		return 0;
	}
	//remove the double quotes
	token.Strip( "\"" );
	//copy the string
	strncpy( (char *) p, token.c_str(), MAX_STRINGFIELD );
	//make sure the string is closed with a zero
	( (char *)p )[MAX_STRINGFIELD - 1] = '\0';
	//
	return 1;
} //end of the function ReadString
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int ReadStructure( idParser *source, structdef_t *def, char *structure ) {
	idToken token;
	fielddef_t *fd;
	void *p;
	int num;

	if ( !source->ExpectTokenString( "{" ) ) {
		return 0;
	}
	while ( 1 )
	{
		if ( !source->ExpectAnyToken( &token ) ) {
			return qfalse;
		}
		//if end of structure
		if ( !strcmp( token.c_str(), "}" ) ) {
			break;
		}
		//find the field with the name
		fd = FindField( def->fields, token.c_str() );
		if ( !fd ) {
			source->Error( "unknown structure field %s", token.c_str() );
			return qfalse;
		} //end if
		if ( fd->type & FT_ARRAY ) {
			num = fd->maxarray;
			if ( !source->ExpectTokenString( "{" ) ) {
				return qfalse;
			}
		} //end if
		else
		{
			num = 1;
		} //end else
		p = ( void * )( structure + fd->offset );
		while ( num-- > 0 )
		{
			if ( fd->type & FT_ARRAY ) {
				if ( source->CheckTokenString( "}" ) ) {
					break;
				}
			} //end if
			switch ( fd->type & FT_TYPE )
			{
			case FT_CHAR:
			{
				if ( !ReadChar( source, fd, p ) ) {
					return qfalse;
				}
				p = (char *) p + sizeof( char );
				break;
			}     //end case
			case FT_INT:
			{
				if ( !ReadNumber( source, fd, p ) ) {
					return qfalse;
				}
				p = (char *) p + sizeof( int );
				break;
			}     //end case
			case FT_FLOAT:
			{
				if ( !ReadNumber( source, fd, p ) ) {
					return qfalse;
				}
				p = (char *) p + sizeof( float );
				break;
			}     //end case
			case FT_STRING:
			{
				if ( !ReadString( source, fd, p ) ) {
					return qfalse;
				}
				p = (char *) p + MAX_STRINGFIELD;
				break;
			}     //end case
			case FT_STRUCT:
			{
				if ( !fd->substruct ) {
					source->Error( "BUG: no sub structure defined" );
					return qfalse;
				}     //end if
				ReadStructure( source, fd->substruct, (char *) p );
				p = (char *) p + fd->substruct->size;
				break;
			}     //end case
			} //end switch
			if ( fd->type & FT_ARRAY ) {
				if ( !source->ExpectAnyToken( &token ) ) {
					return qfalse;
				}
				if ( !strcmp( token.c_str(), "}" ) ) {
					break;
				}
				if ( strcmp( token.c_str(), "," ) ) {
					source->Error( "expected a comma, found %s", token.c_str() );
					return qfalse;
				} //end if
			} //end if
		} //end while
	} //end while
	return qtrue;
} //end of the function ReadStructure
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WriteIndent( FILE *fp, int indent ) {
	while ( indent-- > 0 )
	{
		if ( fprintf( fp, "\t" ) < 0 ) {
			return qfalse;
		}
	} //end while
	return qtrue;
} //end of the function WriteIndent
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WriteFloat( FILE *fp, float value ) {
	char buf[128];
	int l;

	sprintf( buf, "%f", value );
	l = strlen( buf );
	//strip any trailing zeros
	while ( l-- > 1 )
	{
		if ( buf[l] != '0' && buf[l] != '.' ) {
			break;
		}
		if ( buf[l] == '.' ) {
			buf[l] = 0;
			break;
		} //end if
		buf[l] = 0;
	} //end while
	  //write the float to file
	if ( fprintf( fp, "%s", buf ) < 0 ) {
		return 0;
	}
	return 1;
} //end of the function WriteFloat
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WriteStructWithIndent( FILE *fp, structdef_t *def, char *structure, int indent ) {
	int i, num;
	void *p;
	fielddef_t *fd;

	if ( !WriteIndent( fp, indent ) ) {
		return qfalse;
	}
	if ( fprintf( fp, "{\r\n" ) < 0 ) {
		return qfalse;
	}

	indent++;
	for ( i = 0; def->fields[i].name; i++ )
	{
		fd = &def->fields[i];
		if ( !WriteIndent( fp, indent ) ) {
			return qfalse;
		}
		if ( fprintf( fp, "%s\t", fd->name ) < 0 ) {
			return qfalse;
		}
		p = ( void * )( structure + fd->offset );
		if ( fd->type & FT_ARRAY ) {
			num = fd->maxarray;
			if ( fprintf( fp, "{" ) < 0 ) {
				return qfalse;
			}
		} //end if
		else
		{
			num = 1;
		} //end else
		while ( num-- > 0 )
		{
			switch ( fd->type & FT_TYPE )
			{
			case FT_CHAR:
			{
				if ( fprintf( fp, "%d", *(char *) p ) < 0 ) {
					return qfalse;
				}
				p = (char *) p + sizeof( char );
				break;
			}     //end case
			case FT_INT:
			{
				if ( fprintf( fp, "%d", *(int *) p ) < 0 ) {
					return qfalse;
				}
				p = (char *) p + sizeof( int );
				break;
			}     //end case
			case FT_FLOAT:
			{
				if ( !WriteFloat( fp, *(float *)p ) ) {
					return qfalse;
				}
				p = (char *) p + sizeof( float );
				break;
			}     //end case
			case FT_STRING:
			{
				if ( fprintf( fp, "\"%s\"", (char *) p ) < 0 ) {
					return qfalse;
				}
				p = (char *) p + MAX_STRINGFIELD;
				break;
			}     //end case
			case FT_STRUCT:
			{
				if ( !WriteStructWithIndent( fp, fd->substruct, structure, indent ) ) {
					return qfalse;
				}
				p = (char *) p + fd->substruct->size;
				break;
			}     //end case
			} //end switch
			if ( fd->type & FT_ARRAY ) {
				if ( num > 0 ) {
					if ( fprintf( fp, "," ) < 0 ) {
						return qfalse;
					}
				} //end if
				else
				{
					if ( fprintf( fp, "}" ) < 0 ) {
						return qfalse;
					}
				} //end else
			} //end if
		} //end while
		if ( fprintf( fp, "\r\n" ) < 0 ) {
			return qfalse;
		}
	} //end for
	indent--;

	if ( !WriteIndent( fp, indent ) ) {
		return qfalse;
	}
	if ( fprintf( fp, "}\r\n" ) < 0 ) {
		return qfalse;
	}
	return qtrue;
} //end of the function WriteStructWithIndent
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int WriteStructure( FILE *fp, structdef_t *def, char *structure ) {
	return WriteStructWithIndent( fp, def, structure, 0 );
} //end of the function WriteStructure
