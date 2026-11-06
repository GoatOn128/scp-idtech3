#include "precompiled.h"
#include "sys_filesystem.h"

#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

void Sys_Mkdir( const char *path ) {
#ifdef _WIN32
	_mkdir( path );
#else
	mkdir( path, 0777 );
#endif
}

int Sys_ListFiles( const char *directory, const char *extension, idList<idStr> &list ) {
	list.Clear();
	const char *searchDirectory = directory && directory[0] ? directory : ".";
#ifdef _WIN32
	idStr searchPath;
	searchPath.Format( "%s\\*", searchDirectory );
	WIN32_FIND_DATAA entry;
	HANDLE handle = FindFirstFileA( searchPath.c_str(), &entry );
	if ( handle == INVALID_HANDLE_VALUE ) {
		return -1;
	}

	do {
		if ( entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			continue;
		}
		const char *name = entry.cFileName;
#else
	DIR *handle = opendir( searchDirectory );
	if ( !handle ) {
		return -1;
	}

	dirent *entry;
	while ( ( entry = readdir( handle ) ) != nullptr ) {
		idStr path;
		path.Format( "%s/%s", searchDirectory, entry->d_name );
		struct stat status;
		if ( stat( path.c_str(), &status ) != 0 || !S_ISREG( status.st_mode ) ) {
			continue;
		}
		const char *name = entry->d_name;
#endif
		const int nameLength = idStr::Length( name );
		const int extensionLength = extension ? idStr::Length( extension ) : 0;
		if ( extensionLength > nameLength ||
			 ( extensionLength > 0 && idStr::Icmp( name + nameLength - extensionLength, extension ) != 0 ) ) {
			continue;
		}
		list.Append( idStr( name ) );
#ifdef _WIN32
	} while ( FindNextFileA( handle, &entry ) );
	FindClose( handle );
#else
	}
	closedir( handle );
#endif
	return list.Num();
}

const char *Sys_CWD() {
	static char path[4096];
#ifdef _WIN32
	if ( !_getcwd( path, sizeof( path ) ) ) {
#else
	if ( !getcwd( path, sizeof( path ) ) ) {
#endif
		return nullptr;
	}
	return path;
}
