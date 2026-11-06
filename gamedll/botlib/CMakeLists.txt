set(BOTLIB_SOURCES
	be_nav_interface.cpp
	be_ai_char.cpp
	be_ai_chat.cpp
	be_ai_gen.cpp
	be_ai_goal.cpp
	be_ai_move.cpp
	be_ai_weap.cpp
	be_ai_weight.cpp
	be_ea.cpp
	be_interface.cpp
	l_libvar.cpp
	l_log.cpp
	l_struct.cpp
	${CMAKE_SOURCE_DIR}/engine/qcommon/q_shared.cpp)

if ( WIN32 )
	list(APPEND BOTLIB_SOURCES botlib.def)
endif()
	
source_group(TREE "${CMAKE_SOURCE_DIR}" FILES ${BOTLIB_SOURCES})

add_library(botlib SHARED ${BOTLIB_SOURCES})
set_target_properties(botlib PROPERTIES FOLDER "dlls")
set_target_properties(botlib PROPERTIES OUTPUT_NAME botlibx86)
if ( NOT WIN32 )
	set_target_properties(botlib PROPERTIES PREFIX "")
endif()
target_compile_definitions(botlib PRIVATE _LIB BOTLIB)
target_compile_features(botlib PRIVATE cxx_std_17)
target_link_libraries(botlib PUBLIC detour idlib)
if ( MSVC )
	target_link_libraries(botlib PRIVATE winmm)
	target_link_options(botlib PRIVATE /BASE:0x40000000 /MAP:${CMAKE_SOURCE_DIR}/bin/botlibx86.map)
endif()

target_include_directories(botlib PRIVATE "${CMAKE_SOURCE_DIR}/engine/qcommon")

configure_target(botlib)
target_precompile_headers(botlib PRIVATE "${CMAKE_SOURCE_DIR}/gamedll/precompiled.h")