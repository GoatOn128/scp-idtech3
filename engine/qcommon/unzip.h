/* Compatibility include for the repository's MiniZip implementation. */
#include "../../external/minizip/unzip.h"

/* Preserve the old engine naming for central-directory offsets. */
static inline int unzGetCurrentFileInfoPosition( unzFile file, unsigned long *pos ) {
	*pos = unzGetOffset( file );
	return UNZ_OK;
}

static inline int unzSetCurrentFileInfoPosition( unzFile file, unsigned long pos ) {
	return unzSetOffset( file, pos );
}
