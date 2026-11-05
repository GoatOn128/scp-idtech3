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
    l_crc.cpp
    l_libvar.cpp
    l_log.cpp
    l_memory.cpp
    l_precomp.cpp
    l_script.cpp
    l_struct.cpp
    ${CMAKE_SOURCE_DIR}/engine/qcommon/q_math.cpp
    ${CMAKE_SOURCE_DIR}/engine/qcommon/q_shared.cpp)

if ( WIN32 )
    # export GetBotLibAPI through a def file like the game module does
    list(APPEND BOTLIB_SOURCES botlib.def)
endif()
	
source_group(TREE "${CMAKE_SOURCE_DIR}" FILES ${BOTLIB_SOURCES})

# built as its own library, the exe loads it at startup instead of
# carrying the code statically
add_library(botlib SHARED ${BOTLIB_SOURCES})
set_target_properties(botlib PROPERTIES FOLDER "dlls")
target_compile_definitions(botlib PRIVATE _LIB BOTLIB)
target_compile_features(botlib PRIVATE cxx_std_17)
target_link_libraries(botlib PUBLIC detour)
# q_math.cpp/q_shared.cpp include their headers from the engine qcommon dir,
# same setup the game module uses for its copies
target_include_directories(botlib PRIVATE "${CMAKE_SOURCE_DIR}/engine/qcommon")
configure_target(botlib)