#include "pakstuff.h"

#include "cmdlib.h"

int InitPakFile( const char *gameDirectory, const char *unused ) {
	(void)gameDirectory;
	(void)unused;
	return 0;
}

int PakLoadAnyFile( const char *filename, void **buffer ) {
	return LoadFile( filename, buffer );
}
