# The vendored idlib from the Doom 3 BFG GPL release, trimmed down to the
# parts the engine uses: math, containers, strings, lexer and parser.
# This is third-party code, so it is not held to the project's /W4 bar.

set( IDLIB_SOURCES
    CmdArgs.cpp
    Heap.cpp
    Lexer.cpp
    Parser.cpp
    Str.cpp
    Token.cpp
    bv/Bounds.cpp
    containers/HashIndex.cpp
    math/Angles.cpp
    math/Math.cpp
    math/Matrix.cpp
    math/Plane.cpp
    math/Quat.cpp
    math/Rotation.cpp
    math/Vector.cpp )

add_library( idlib STATIC ${IDLIB_SOURCES} )

# let includers write #include "idlib/..." while the idlib sources keep their
# own relative includes
add_dependencies( idlib )
target_include_directories( idlib PUBLIC ${CMAKE_SOURCE_DIR}/engine )

if ( MSVC )
	target_compile_definitions( idlib PRIVATE
		WIN32 _WINDOWS _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS
		$<$<CONFIG:Debug>:_DEBUG;DEBUG>
		$<$<NOT:$<CONFIG:Debug>>:NDEBUG>)
	target_compile_options( idlib PRIVATE
		/FS /W3
		/wd4100 /wd4127 /wd4244 /wd4245 /wd4267 /wd4389 /wd4702 /wd4706 /wd4996)
else()
	target_compile_definitions( idlib PRIVATE
		$<$<CONFIG:Debug>:_DEBUG;DEBUG>
		$<$<NOT:$<CONFIG:Debug>>:NDEBUG>)
	if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
		target_compile_options( idlib PRIVATE -Wno-writable-strings -Wno-deprecated-writable-strings )
	elseif ( CMAKE_CXX_COMPILER_ID MATCHES "GNU" )
		target_compile_options( idlib PRIVATE -Wno-write-strings )
	endif()
endif()

# platform wide macros, see PLATFORM_DEFINES in the top level CMakeLists
target_compile_definitions( idlib PRIVATE ${PLATFORM_DEFINES} )

set_output_directory( idlib )
