/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code.

===========================================================================
*/

#ifndef __PARSETREE_H__
#define __PARSETREE_H__

#include "Parser.h"

class idParseValue {
public:
	idParseValue() {}

	const char *	GetName() const { return name.c_str(); }
	const char *	GetTopValue() const { return values.Num() ? values[0].c_str() : NULL; }
	const char *	GetValue( int index ) const { return index >= 0 && index < values.Num() ? values[index].c_str() : NULL; }
	int				GetNumValues() const { return values.Num(); }
	bool			IsList() const { return list; }

private:
	friend class idParseGroup;
	friend class idParseTree;
	idStr			name;
	idList<idStr>	values;
	bool			list = false;
};

class idParseGroup {
public:
	const char *			GetName() const { return name.c_str(); }
	int						GetNumPairs() const { return pairs.Num(); }
	const idParseValue *	GetPair( int index ) const { return index >= 0 && index < pairs.Num() ? pairs[index] : NULL; }
	const idParseValue *	FindPair( const char *key ) const;
	int						GetNumSubGroups() const { return groups.Num(); }
	const idParseGroup *	GetSubGroup( int index ) const { return index >= 0 && index < groups.Num() ? groups[index] : NULL; }

private:
	friend class idParseTree;
	explicit idParseGroup( const char *name );
	~idParseGroup();

	idStr					name;
	idList<idParseValue *>	pairs;
	idList<idParseGroup *>	groups;
};

class idParseTree {
public:
	idParseTree();
	~idParseTree();

	bool					Parse( const char *text, int length, const char *name );
	const idParseGroup *	GetRoot() const { return root; }

private:
	bool					ParseGroup( idParser &parser, idParseGroup *group, bool nested );
	void					ReadLineValue( idParser &parser, idToken &token, idStr &value );
	bool					ReadList( idParser &parser, idParseValue *value );

	idParseGroup *			root;
};

#endif
