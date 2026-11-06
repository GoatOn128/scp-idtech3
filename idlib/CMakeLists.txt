file( GLOB_RECURSE IDLIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp )
list( FILTER IDLIB_SOURCES EXCLUDE REGEX "/(CommandLink|ParallelJobList|math/Lcp)\\.cpp$" )

add_library( idlib STATIC ${IDLIB_SOURCES} )
source_group( TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${IDLIB_SOURCES} )
set_target_properties( idlib PROPERTIES FOLDER "libs" )

target_include_directories( idlib PUBLIC ${CMAKE_SOURCE_DIR}/engine )

configure_target( idlib )

if ( MSVC )
	target_compile_options( idlib PRIVATE /wd4100 /wd4127 )
endif()
