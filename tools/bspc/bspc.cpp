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

#include "navmesh_builder.h"

#include <stdio.h>
#include <stdlib.h>

namespace {
class BspcApplication final {
public:
	int Run( int argc, char **argv ) const {
		NavBuildOptions options;
		idStr extension;
		const int parseResult = ParseArguments( argc, argv, options, extension );
		if ( parseResult != 0 )
			return parseResult == 1 ? 0 : 2;
		idStr error;
		NavMeshBuilder builder( options );
		if ( !builder.Build( error ) ) {
			fprintf( stderr, "bspc: %s\n", error.c_str() );
			return 1;
		}
		printf( "wrote %s\n", options.outputPath.c_str() );
		return 0;
	}

private:
	static void Usage() {
		printf( "BSP navmesh compiler\n"
				"usage: bspc -bsp2nav <map.bsp> -cfg <aascfg.c> [-ext suffix] [-output directory]\n"
				"       -bsp2aas is accepted as a compatibility alias for -bsp2nav\n" );
	}

	static int ParseArguments( int argc, char **argv, NavBuildOptions &options, idStr &extension ) {
		idStr bsp;
		for ( int i = 1; i < argc; ++i ) {
			const idStr arg = argv[i];
			auto value = [&]( const char *name ) -> const char * {
				if ( i + 1 >= argc ) {
					fprintf( stderr, "missing value for %s\n", name );
					return nullptr;
				}
				return argv[++i];
			};
			const char *argument = nullptr;
			if ( !idStr::Cmp( arg, "-bsp2nav" ) || !idStr::Cmp( arg, "-bsp2aas" ) ) {
				if ( !( argument = value( arg.c_str() ) ) ) return 2;
				bsp = argument;
			} else if ( arg == "-cfg" ) {
				if ( !( argument = value( "-cfg" ) ) ) return 2;
				options.configPath = argument;
			} else if ( arg == "-ext" ) {
				if ( !( argument = value( "-ext" ) ) ) return 2;
				extension = argument;
			} else if ( arg == "-output" ) {
				if ( !( argument = value( "-output" ) ) ) return 2;
				options.outputPath = argument;
			} else if ( arg == "-threads" ) {
				if ( !value( "-threads" ) ) return 2;
			} else if ( arg == "-optimize" ) {
			} else if ( arg == "-help" || arg == "--help" || arg == "/?" ) {
				Usage();
				return 1;
			} else {
				fprintf( stderr, "unknown option: %s\n", arg.c_str() );
				Usage();
				return 2;
			}
		}
		if ( bsp.IsEmpty() || options.configPath.IsEmpty() ) {
			Usage();
			return 2;
		}
		options.bspPath = bsp;
		if ( options.outputPath.IsEmpty() ) {
			const int slash = Max( bsp.Last( '/' ), bsp.Last( '\\' ) );
			if ( slash >= 0 ) options.outputPath = bsp.Left( slash );
		}
		const int dot = bsp.Last( '.' );
		const int slash = Max( bsp.Last( '/' ), bsp.Last( '\\' ) );
		const idStr stem = dot > slash ? bsp.Left( dot ) : bsp;
		if ( !options.outputPath.IsEmpty() ) {
			const int stemSlash = Max( stem.Last( '/' ), stem.Last( '\\' ) );
			options.outputPath.AppendPath( stem.Mid( stemSlash + 1, stem.Length() - stemSlash - 1 ).c_str() );
		} else {
			options.outputPath = stem.Mid( slash + 1, stem.Length() - slash - 1 );
		}
		options.outputPath += extension;
		options.outputPath += ".nav";
		options.agentIndex = extension == "_b1" ? 1 : 0;
		return 0;
	}
};
}

int main( int argc, char **argv ) {
	return BspcApplication().Run( argc, argv );
}
