cmake_minimum_required(VERSION 3.25)

project(WolfSP LANGUAGES C CXX)

# The Windows toolchain is intentionally 32-bit.  The original asset tools
# use 32-bit file formats and are not supported as x64 executables.
if ( WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8 )
	message(FATAL_ERROR "Windows builds must use the Win32/x86 toolchain; x64 is unsupported")
endif()

# the AI state machine uses scoped enums: build as C++14 everywhere
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ( WIN32 )
	enable_language( RC )
endif()

function(set_output_directory target)
	set(output_dir "$<1:>${CMAKE_SOURCE_DIR}/bin")
	# optional argument: subdirectory of bin/ the target's binaries belong to
	if ( ARGN )
		set(output_dir "${output_dir}/${ARGV1}")
	endif()
	set_target_properties(${target} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${output_dir}"
		LIBRARY_OUTPUT_DIRECTORY "${output_dir}"
		ARCHIVE_OUTPUT_DIRECTORY "$<1:>${CMAKE_BINARY_DIR}/lib"
		PDB_OUTPUT_DIRECTORY "${output_dir}"
		COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/pdb")
	
	foreach(configuration Debug Release RelWithDebInfo MinSizeRel)
		string(TOUPPER "${configuration}" configuration_upper)
		set_target_properties(${target} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_${configuration_upper} "${output_dir}"
			LIBRARY_OUTPUT_DIRECTORY_${configuration_upper} "${output_dir}"
			ARCHIVE_OUTPUT_DIRECTORY_${configuration_upper} "${CMAKE_BINARY_DIR}/lib/${configuration}"
			PDB_OUTPUT_DIRECTORY_${configuration_upper} "${output_dir}"
			COMPILE_PDB_OUTPUT_DIRECTORY_${configuration_upper} "${CMAKE_BINARY_DIR}/pdb/${configuration}")
	endforeach()
endfunction()

function(configure_target target)
	if ( MSVC )
		target_compile_definitions(${target} PRIVATE
			WIN32 _WINDOWS _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS
			$<$<CONFIG:Debug>:_DEBUG;DEBUG>
			$<$<NOT:$<CONFIG:Debug>>:NDEBUG>)
		target_compile_options(${target} PRIVATE
			/FS
			$<$<CONFIG:Debug>:/W3>
			$<$<NOT:$<CONFIG:Debug>>:/W4>)
	else()
		target_compile_definitions(${target} PRIVATE
			$<$<CONFIG:Debug>:_DEBUG;DEBUG>
			$<$<NOT:$<CONFIG:Debug>>:NDEBUG>)
		if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
			target_compile_options(${target} PRIVATE
				-Wno-writable-strings -Wno-deprecated-writable-strings
				-Wno-c++11-compat-deprecated-writable-strings)
		elseif ( CMAKE_CXX_COMPILER_ID MATCHES "GNU" )
			target_compile_options(${target} PRIVATE -Wno-write-strings)
		endif()
	endif()

	target_compile_definitions(${target} PRIVATE ${PLATFORM_DEFINES})
	target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/idlib)
	set_output_directory(${target} ${ARGN})
endfunction()

if ( MSVC )
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Platform defines
set(PLATFORM_DEFINES "MAC_STATIC=")
if ( WIN32 )
	list(APPEND PLATFORM_DEFINES "QDECL=__cdecl")
	string(ASCII 92 Q_BACKSLASH)
	if ( MSVC )
		string(CONCAT Q_PATH_SEP "PATH_SEP='" "${Q_BACKSLASH}" "${Q_BACKSLASH}" "'")
	else()
		string(CONCAT Q_PATH_SEP "PATH_SEP='" "${Q_BACKSLASH}" "${Q_BACKSLASH}" "${Q_BACKSLASH}" "${Q_BACKSLASH}" "'")
	endif()
	list(APPEND PLATFORM_DEFINES "${Q_PATH_SEP}")
list(APPEND PLATFORM_DEFINES "CPUSTRING=\"win-x86$<$<CONFIG:Debug>:-debug>\"")
elseif ( APPLE )
	list(APPEND PLATFORM_DEFINES "QDECL=" "PATH_SEP='/'")
	list(APPEND PLATFORM_DEFINES "CPUSTRING=\"macos-${CMAKE_SYSTEM_PROCESSOR}\"")
else()
	list(APPEND PLATFORM_DEFINES "QDECL=" "PATH_SEP='/'")
	if ( CMAKE_SYSTEM_PROCESSOR MATCHES "^i\\.86$" )
		set(Q_CPUSTRING "linux-i386")
	elseif ( CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$" )
		set(Q_CPUSTRING "linux-x86_64")
	elseif ( CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$" )
		set(Q_CPUSTRING "linux-arm64")
	else()
		set(Q_CPUSTRING "linux-${CMAKE_SYSTEM_PROCESSOR}")
	endif()
	list(APPEND PLATFORM_DEFINES "CPUSTRING=\"${Q_CPUSTRING}\"")
endif()

option(HARD_LINKED_MODULES "Define the GAME/CGAME/UI/BOTLIB_HARD_LINKED macros so the modules bind the engine's Com_Error and Com_Printf" OFF)
if ( HARD_LINKED_MODULES )
	list(APPEND PLATFORM_DEFINES GAME_HARD_LINKED CGAME_HARD_LINKED UI_HARD_LINKED BOTLIB_HARD_LINKED)
endif()

# idLib
add_subdirectory(idlib)

# External
add_subdirectory(external)

if ( NOT MSVC )
	get_target_property( detour_compile_options detour COMPILE_OPTIONS )
	if ( detour_compile_options )
		list( REMOVE_ITEM detour_compile_options "/wd4127" )
		set_target_properties( detour PROPERTIES COMPILE_OPTIONS "${detour_compile_options}" )
	endif()
endif()

# Tools
add_subdirectory(tools/bspc)
add_subdirectory(tools/extractfuncs)
add_subdirectory(tools/q3map)

# GameDLLs
add_subdirectory(gamedll/botlib)
add_subdirectory(gamedll/game)
add_subdirectory(gamedll/cgame)
add_subdirectory(gamedll/ui)

# Engine
add_subdirectory(engine)
