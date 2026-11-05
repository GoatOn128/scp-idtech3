# idlib builds as a standalone static library from the vendored idlib tree
file( GLOB_RECURSE IDLIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp )

add_library( idlib STATIC ${IDLIB_SOURCES} )
source_group( TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${IDLIB_SOURCES} )
set_target_properties( idlib PROPERTIES FOLDER "libs" )

# consumers include idlib headers as "idlib/..." from the engine root
target_include_directories( idlib PUBLIC ${CMAKE_SOURCE_DIR}/engine )

configure_target( idlib )

# idlib is header/template heavy BFG code: silence the two warnings it trips
# constantly by design rather than mangling the upstream sources
if ( MSVC )
	target_compile_options( idlib PRIVATE /wd4100 /wd4127 )
endif()
