/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code.

===========================================================================
*/

#include "precompiled.h"
#include "ParseTree.h"

static char parseTreeOpenBrace[] = "{";
static char parseTreeCloseBrace[] = "}";
static char parseTreeOpenBracket[] = "[";
static char parseTreeCloseBracket[] = "]";
static punctuation_t parseTreePunctuations[] = {
	{ parseTreeOpenBrace, P_BRACEOPEN },
	{ parseTreeCloseBrace, P_BRACECLOSE },
	{ parseTreeOpenBracket, P_SQBRACKETOPEN },
	{ parseTreeCloseBracket, P_SQBRACKETCLOSE },
	{ NULL, 0 }
};

idParseGroup::idParseGroup( const char *groupName ) :
	name( groupName )
{
}

idParseGroup::~idParseGroup() {
	for ( int i = 0; i < pairs.Num(); i++ ) {
		delete pairs[i];
	}
	for ( int i = 0; i < groups.Num(); i++ ) {
		delete groups[i];
	}
}

const idParseValue *idParseGroup::FindPair( const char *key ) const {
	for ( int i = 0; i < pairs.Num(); i++ ) {
		if ( pairs[i]->name.Icmp( key ) == 0 ) {
			return pairs[i];
		}
	}
	return NULL;
}

idParseTree::idParseTree() :
	root( new idParseGroup( "Top Level" ) )
{
}

idParseTree::~idParseTree() {
	delete root;
}

bool idParseTree::Parse( const char *text, int length, const char *sourceName ) {
	idParser parser( LEXFL_NOERRORS | LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_NODOLLARPRECOMPILE );

	delete root;
	root = new idParseGroup( "Top Level" );
	parser.SetPunctuations( parseTreePunctuations );
	if ( !parser.LoadMemory( text, length, sourceName ) ) {
		parser.SetPunctuations( NULL );
		return false;
	}

	bool parsed = ParseGroup( parser, root, false );
	parser.FreeSource();
	parser.SetPunctuations( NULL );
	return parsed;
}

void idParseTree::ReadLineValue( idParser &parser, idToken &token, idStr &value ) {
	value = token;
	while ( parser.ReadTokenOnLine( &token ) ) {
		if ( token.type == TT_PUNCTUATION && ( token == "}" || token == "]" ) ) {
			parser.UnreadToken( &token );
			break;
		}
		value.Append( " " );
		value.Append( token );
	}
}

bool idParseTree::ReadList( idParser &parser, idParseValue *value ) {
	idToken token;

	while ( parser.ReadToken( &token ) ) {
		if ( token.type == TT_PUNCTUATION && token == "]" ) {
			return true;
		}
		if ( token.type == TT_PUNCTUATION && ( token == "{" || token == "}" || token == "[" ) ) {
			return false;
		}

		idStr item;
		ReadLineValue( parser, token, item );
		value->values.Append( item );
		if ( token.type == TT_PUNCTUATION && token == "]" ) {
			return parser.CheckTokenString( "]" );
		}
	}
	return false;
}

bool idParseTree::ParseGroup( idParser &parser, idParseGroup *group, bool nested ) {
	idToken token;

	while ( parser.ReadToken( &token ) ) {
		if ( token.type == TT_PUNCTUATION && token == "}" ) {
			return nested;
		}
		if ( token.type == TT_PUNCTUATION && ( token == "{" || token == "[" || token == "]" ) ) {
			return false;
		}

		idStr name( token );
		if ( !parser.ReadToken( &token ) ) {
			return false;
		}

		if ( token.type == TT_PUNCTUATION && token == "{" ) {
			idParseGroup *subgroup = new idParseGroup( name );
			group->groups.Append( subgroup );
			if ( !ParseGroup( parser, subgroup, true ) ) {
				return false;
			}
		} else {
			idParseValue *pair = new idParseValue;
			pair->name = name;
			group->pairs.Append( pair );
			if ( token.type == TT_PUNCTUATION && token == "[" ) {
				pair->list = true;
				if ( !ReadList( parser, pair ) ) {
					return false;
				}
			} else if ( token.type == TT_PUNCTUATION && ( token == "}" || token == "]" || token == "{" ) ) {
				return false;
			} else {
				idStr value;
				ReadLineValue( parser, token, value );
				pair->values.Append( value );
			}
		}
	}

	return !nested;
}
