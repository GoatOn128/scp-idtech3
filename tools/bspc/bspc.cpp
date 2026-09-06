#include "navmesh_builder.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {
void Usage() {
	std::cout << "RTCW BSP navmesh compiler\n"
		<< "usage: bspc -bsp2nav <map.bsp> -cfg <aascfg.c> [-ext suffix] [-output directory]\n"
		<< "       -bsp2aas is accepted as a compatibility alias for -bsp2nav\n";
}
}

int main( int argc, char **argv ) {
	std::filesystem::path bsp;
	std::filesystem::path cfg;
	std::filesystem::path output;
	std::string extension;

	for ( int i = 1; i < argc; ++i ) {
		const std::string arg = argv[i];
		auto value = [&]( const char *name ) -> const char * {
			if ( i + 1 >= argc ) {
				std::cerr << "missing value for " << name << '\n';
				std::exit( 2 );
			}
			return argv[++i];
		};
		if ( arg == "-bsp2nav" || arg == "-bsp2aas" ) {
			bsp = value( arg.c_str() );
		} else if ( arg == "-cfg" ) {
			cfg = value( "-cfg" );
		} else if ( arg == "-ext" ) {
			extension = value( "-ext" );
		} else if ( arg == "-output" ) {
			output = value( "-output" );
		} else if ( arg == "-threads" ) {
			(void)value( "-threads" ); // Recast owns its build scheduling.
		} else if ( arg == "-optimize" ) {
			// Recast contour and polygon generation is always optimized.
		} else if ( arg == "-help" || arg == "--help" || arg == "/?" ) {
			Usage();
			return 0;
		} else {
			std::cerr << "unknown option: " << arg << '\n';
			Usage();
			return 2;
		}
	}

	if ( bsp.empty() || cfg.empty() ) {
		Usage();
		return 2;
	}
	if ( output.empty() ) {
		output = bsp.parent_path();
	}
	std::filesystem::path destination = output / ( bsp.stem().string() + extension + ".nav" );
	NavBuildOptions options{ bsp, cfg, destination, extension == "_b1" ? 1 : 0 };
	std::string error;
	if ( !BuildNavMesh( options, error ) ) {
		std::cerr << "bspc: " << error << '\n';
		return 1;
	}
	std::cout << "wrote " << destination.string() << '\n';
	return 0;
}
