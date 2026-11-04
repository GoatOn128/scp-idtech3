cmake_minimum_required(VERSION 3.25)

project(WolfSP LANGUAGES C CXX)

if ( WIN32 )
    enable_language( RC )
endif()

function(set_output_directory target)
	set_target_properties(${target} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "$<1:>${CMAKE_SOURCE_DIR}/bin"
		LIBRARY_OUTPUT_DIRECTORY "$<1:>${CMAKE_SOURCE_DIR}/bin"
		ARCHIVE_OUTPUT_DIRECTORY "$<1:>${CMAKE_BINARY_DIR}/lib"
		PDB_OUTPUT_DIRECTORY "$<1:>${CMAKE_SOURCE_DIR}/bin"
		COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/pdb")
	
	foreach(configuration Debug Release RelWithDebInfo MinSizeRel)
		string(TOUPPER "${configuration}" configuration_upper)
		set_target_properties(${target} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_${configuration_upper} "$<1:>${CMAKE_SOURCE_DIR}/bin"
			LIBRARY_OUTPUT_DIRECTORY_${configuration_upper} "$<1:>${CMAKE_SOURCE_DIR}/bin"
			ARCHIVE_OUTPUT_DIRECTORY_${configuration_upper} "$<1:>${CMAKE_BINARY_DIR}/lib/${configuration}"
			PDB_OUTPUT_DIRECTORY_${configuration_upper} "$<1:>${CMAKE_SOURCE_DIR}/bin"
			COMPILE_PDB_OUTPUT_DIRECTORY_${configuration_upper} "${CMAKE_BINARY_DIR}/pdb/${configuration}")
	endforeach()
endfunction()

function(configure_target target)
	if ( MSVC )
		target_compile_definitions(${target} PRIVATE
			WIN32 _WINDOWS _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS
			$<CONFIG:Debug>:_DEBUG;DEBUG>
			$<NOT:$<CONFIG:Debug>>:NDEBUG>)
		target_compile_options(${target} PRIVATE
			/FS
			$<CONFIG:Debug>:/W3>
			$<NOT:$<CONFIG:Debug>>:/W4>)
	else()
		target_compile_definitions(${target} PRIVATE
			$<CONFIG:Debug>:_DEBUG;DEBUG>
			$<NOT:$<CONFIG:Debug>>:NDEBUG>)
	endif()
	set_output_directory(${target})
endfunction()

if ( MSVC )
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# External
add_subdirectory(external)

# Tools (Windows only, they carry win32 specific code)
if ( WIN32 )
	add_subdirectory(tools/bspc)
	add_subdirectory(tools/extractfuncs)
endif()

# GameDLLs
add_subdirectory(gamedll/botlib)
add_subdirectory(gamedll/game)
add_subdirectory(gamedll/cgame)
add_subdirectory(gamedll/ui)

# Engine
add_subdirectory(engine)
