#ifndef Q3MAP_PAKSTUFF_H
#define Q3MAP_PAKSTUFF_H

int InitPakFile( const char *gameDirectory, const char *unused );
int PakLoadAnyFile( const char *filename, void **buffer );

#endif
