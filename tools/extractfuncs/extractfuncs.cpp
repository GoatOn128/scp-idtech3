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

#pragma hdrstop

#include "precompiled.h"

#include <stdio.h>
#include <limits.h>
#include <filesystem>

const int MAX_DECLARATION_LENGTH = 1024;

struct FunctionRecord {
	idStr name;
	idStr replacement;
	idStr filename;
	idStr declaration;
};

class FunctionRegistry {
public:
	void Add( const char *name, const char *filename, const idList<idToken, TAG_EXTRACTFUNCS> &declaration ) {
		if ( Find( name ) != NULL || IsExcluded( name ) ) {
			return;
		}

		FunctionRecord record;
		record.name = name;
		char replacement[16];
		idStr::snPrintf( replacement, sizeof( replacement ), "F%d", functions.Num() );
		record.replacement = replacement;
		record.filename = filename;
		for ( int i = 0; i < declaration.Num(); ++i ) {
			if ( i != 0 ) {
				record.declaration.Append( ' ' );
			}
			record.declaration.Append( declaration[i].c_str() );
		}
		if ( record.declaration.Length() >= 6 &&
			 !record.declaration.Icmpn( "endif ", 6 ) ) {
			record.declaration = record.declaration.Mid( 6, record.declaration.Length() - 6 );
		}
		functions.Append( record );
	}

	const FunctionRecord *Find( const char *name ) const {
		for ( int i = 0; i < functions.Num(); ++i ) {
			if ( !strcmp( functions[i].name.c_str(), name ) ) {
				return &functions[i];
			}
		}
		return NULL;
	}

	int Num() const {
		return functions.Num();
	}

	const FunctionRecord &operator[]( int index ) const {
		return functions[index];
	}

private:
	static bool IsExcluded( const char *name ) {
		if ( !strcmp( name, "GetBotAPI" ) || !strcmp( name, "GetGameAPI" ) ||
			 !strcmp( name, "main" ) || !strcmp( name, "WinMain" ) ) {
			return true;
		}

		if ( !idStr::Icmp( name, "BoxOnSide" ) ) {
			return true;
		}

		if ( !idStr::Icmp( name, "qmax" ) || !idStr::Icmp( name, "qmin" ) ) {
			return true;
		}
		return false;
	}

	idList<FunctionRecord, TAG_EXTRACTFUNCS> functions;
};

class SourceReader {
public:
	static bool Read( const char *filename, idStr &text ) {
		FILE *file = fopen( filename, "rb" );
		if ( file == NULL ) {
			fprintf( stderr, "error opening %s\n", filename );
			return false;
		}

		char buffer[4096];
		size_t bytesRead;
		while ( ( bytesRead = fread( buffer, 1, sizeof( buffer ), file ) ) != 0 ) {
			if ( text.Length() > INT_MAX - static_cast<int>( bytesRead ) ) {
				fclose( file );
				fprintf( stderr, "source file is too large: %s\n", filename );
				return false;
			}
			text.Append( buffer, static_cast<int>( bytesRead ) );
		}
		fclose( file );
		return true;
	}
};

class FunctionScanner {
public:
	explicit FunctionScanner( FunctionRegistry &registry ) : registry( registry ) {}

	bool Scan( const char *filename ) {
		if ( !idStr::Icmp( filename, "bg_lib.c" ) ) {
			return true;
		}

		idStr sourceText;
		if ( !SourceReader::Read( filename, sourceText ) ) {
			return false;
		}
		idLexer lexer;
		if ( !lexer.LoadMemory( sourceText.c_str(), sourceText.Length(), filename ) ) {
			fprintf( stderr, "error parsing %s\n", filename );
			return false;
		}

		idList<idToken, TAG_EXTRACTFUNCS> tokens;
		idToken token;
		idToken previous;
		int braceDepth = 0;
		bool isStatic = false;
		while ( lexer.ReadToken( &token ) ) {
			tokens.Append( token );
			if ( token.type == TT_PUNCTUATION ) {
				const char punctuation = token.c_str()[0];
				if ( punctuation == ';' ) {
					isStatic = false;
				} else if ( punctuation == '{' ) {
					++braceDepth;
				} else if ( punctuation == '}' ) {
					braceDepth = Max( 0, braceDepth - 1 );
				} else if ( punctuation == '(' && braceDepth == 0 &&
							previous.type == TT_NAME ) {
					if ( !ScanDeclaration( lexer, tokens, filename, previous,
										 isStatic, braceDepth ) ) {
						return false;
					}
				}
			}
			if ( token.type == TT_NAME && !strcmp( token.c_str(), "static" ) ) {
				isStatic = true;
			}
			previous = token;
		}
		return true;
	}

private:
	bool ScanDeclaration( idLexer &lexer, idList<idToken, TAG_EXTRACTFUNCS> &tokens,
						  const char *filename, const idToken &functionName,
						  bool isStatic, int &braceDepth ) {
		const int nameIndex = tokens.Num() - 1;
		int declarationStart = nameIndex;
		while ( declarationStart > 0 ) {
			const idToken &token = tokens[declarationStart - 1];
			if ( token.type != TT_NAME && token[0] != '*' ) {
				break;
			}
			--declarationStart;
		}

		idList<idToken, TAG_EXTRACTFUNCS> declaration;
		for ( int i = declarationStart; i < tokens.Num(); ++i ) {
			declaration.Append( tokens[i] );
		}

		idToken token;
		int parenthesisDepth = 1;
		while ( lexer.ReadToken( &token ) ) {
			declaration.Append( token );
			tokens.Append( token );
			if ( token[0] == '(' ) {
				++parenthesisDepth;
			} else if ( token[0] == ')' && --parenthesisDepth == 0 ) {
				if ( !lexer.ReadToken( &token ) ) {
					return true;
				}
				if ( token[0] == '{' ) {
					if ( !isStatic && MayScrewUp( functionName.c_str() ) ) {
						registry.Add( functionName.c_str(), filename, declaration );
					}
					++braceDepth;
				}
				return true;
			}
		}
		return true;
	}

	static bool MayScrewUp( const char *name ) {
		return strcmp( name, "GetBotAPI" ) && strcmp( name, "GetGameAPI" ) &&
			strcmp( name, "main" ) && strcmp( name, "WinMain" );
	}

	FunctionRegistry &registry;
};

class FunctionTableWriter {
public:
	explicit FunctionTableWriter( const FunctionRegistry &registry ) : registry( registry ) {}

	bool Write( const char *filename, const char *declarationFilename ) const {
		bool updated = WriteFile( filename, "g_funcs.tmp", false );
		updated = WriteFile( declarationFilename, "g_func_decs.tmp", true ) || updated;
		if ( updated ) {
			printf( "Updated the function table, recompile required.\n" );
		}
		return updated;
	}

private:
	bool WriteFile( const char *filename, const char *temporary, bool declarations ) const {
		FILE *file = fopen( temporary, "wb" );
		if ( file == NULL ) {
			fprintf( stderr, "error opening %s\n", temporary );
			return false;
		}
		for ( int i = registry.Num() - 1; i >= 0; --i ) {
			const FunctionRecord &record = registry[i];
			if ( declarations ) {
				fprintf( file, "extern %s;\n", record.declaration.c_str() );
			} else {
				fprintf( file, "{\"%s\", (byte *)%s},\n",
					record.name.c_str(), record.name.c_str() );
			}
		}
		if ( !declarations ) {
			fputs( "{0, 0}\n", file );
		}
		fclose( file );
		return ReplaceIfChanged( filename, temporary );
	}

	static bool ReplaceIfChanged( const char *filename, const char *temporary ) {
		idStr generated;
		idStr existing;
		if ( !SourceReader::Read( temporary, generated ) ) {
			return false;
		}
		if ( SourceReader::Read( filename, existing ) &&
			 generated.Length() == existing.Length() &&
			 !idStr::Icmp( generated.c_str(), existing.c_str() ) ) {
			remove( temporary );
			return false;
		}
		remove( filename );
		if ( rename( temporary, filename ) != 0 ) {
			fprintf( stderr, "error replacing %s\n", filename );
			return false;
		}
		return true;
	}

	const FunctionRegistry &registry;
};

class SourceRewriter {
public:
	explicit SourceRewriter( const FunctionRegistry &registry ) : registry( registry ) {}

	bool Rewrite( const char *oldFilename, const char *newFilename ) const {
		idStr sourceText;
		if ( !SourceReader::Read( oldFilename, sourceText ) ) {
			return false;
		}
		idLexer lexer;
		if ( !lexer.LoadMemory( sourceText.c_str(), sourceText.Length(), oldFilename ) ) {
			return false;
		}
		FILE *file = fopen( newFilename, "wb" );
		if ( file == NULL ) {
			fprintf( stderr, "error opening %s\n", newFilename );
			return false;
		}
		int sourceOffset = 0;
		idToken token;
		while ( lexer.ReadToken( &token ) ) {
			const int tokenStart = lexer.GetLastWhiteSpaceEnd();
			const int tokenEnd = lexer.GetFileOffset();
			WriteWhitespace( file, sourceText, sourceOffset, tokenStart );
			const FunctionRecord *record =
				token.type == TT_NAME ? registry.Find( token.c_str() ) : NULL;
			if ( record != NULL ) {
				fputs( record->replacement.c_str(), file );
			} else {
				fwrite( sourceText.c_str() + tokenStart, 1, tokenEnd - tokenStart, file );
			}
			sourceOffset = tokenEnd;
		}
		WriteWhitespace( file, sourceText, sourceOffset, sourceText.Length() );
		fclose( file );
		return true;
	}

private:
	static void WriteWhitespace( FILE *file, const idStr &text, int start, int end ) {
		for ( int i = start; i < end; ++i ) {
			if ( text[i] != '\r' ) {
				fputc( text[i], file );
			}
		}
	}

	const FunctionRegistry &registry;
};

class Extractor {
public:
	bool Scan( const char *filename ) {
		return scanner.Scan( filename );
	}

	bool WriteTables( const char *functions, const char *declarations ) const {
		return writer.Write( functions, declarations );
	}

private:
	FunctionRegistry registry;
	FunctionScanner scanner = FunctionScanner( registry );
	FunctionTableWriter writer = FunctionTableWriter( registry );
};

static bool Matches( const char *pattern, const char *value ) {
	if ( *pattern == '\0' ) {
		return *value == '\0';
	}
	if ( *pattern == '*' ) {
		return Matches( pattern + 1, value ) ||
			( *value != '\0' && Matches( pattern, value + 1 ) );
	}
	return *value != '\0' &&
		( *pattern == '?' || *pattern == *value ) &&
		Matches( pattern + 1, value + 1 );
}

static bool ScanFilter( Extractor &extractor, const char *filter ) {
	const std::filesystem::path filterPath( filter );
	const std::filesystem::path directory =
		filterPath.has_parent_path() ? filterPath.parent_path() : ".";
	const std::string filenamePattern = filterPath.filename().string();
	std::error_code error;
	if ( !std::filesystem::is_directory( directory, error ) ) {
		fprintf( stderr, "No source files match %s\n", filter );
		return false;
	}

	bool found = false;
	bool success = true;
	for ( const std::filesystem::directory_entry &entry :
		  std::filesystem::directory_iterator( directory, error ) ) {
		if ( error ) {
			break;
		}
		if ( !entry.is_regular_file( error ) ||
			 !Matches( filenamePattern.c_str(), entry.path().filename().string().c_str() ) ||
			 entry.path().filename() == "bg_lib.cpp" ) {
			continue;
		}
		found = true;
		const std::string sourcePath = entry.path().string();
		success = extractor.Scan( sourcePath.c_str() ) && success;
	}
	if ( !found || error ) {
		fprintf( stderr, "No source files match %s\n", filter );
		return false;
	}
	return success;
}

int main( int argc, char **argv ) {
	if ( argc < 2 ) {
		fprintf( stderr, "USAGE: extractfuncs <file filter> [<file filter> ...]\n" );
		return 1;
	}

	const char *functionsFilename = "g_funcs.h";
	const char *declarationsFilename = "g_func_decs.h";
	int firstInput = 1;
	if ( !idStr::Icmp( argv[1], "-o" ) ) {
		if ( argc < 5 ) {
			fprintf( stderr, "USAGE: extractfuncs [-o <funcs> <func_desc>] <file> [...]\n" );
			return 1;
		}
		functionsFilename = argv[2];
		declarationsFilename = argv[3];
		firstInput = 4;
	}

	Extractor extractor;
	for ( int i = firstInput; i < argc; ++i ) {
		if ( !ScanFilter( extractor, argv[i] ) ) {
			return 1;
		}
	}
	return extractor.WriteTables( functionsFilename, declarationsFilename ) ? 0 : 1;
}
