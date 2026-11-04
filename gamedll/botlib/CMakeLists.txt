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
    l_struct.cpp)
	
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${BOTLIB_SOURCES})

add_library(botlib STATIC ${BOTLIB_SOURCES})
set_target_properties(botlib PROPERTIES FOLDER "libs")
target_compile_definitions(botlib PRIVATE _LIB BOTLIB)
target_compile_features(botlib PRIVATE cxx_std_17)
target_link_libraries(botlib PUBLIC detour)
configure_target(botlib)