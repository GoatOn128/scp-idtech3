set(UI_SOURCES
    ../game/bg_lib.cpp
    ../game/bg_misc.cpp
    ${CMAKE_SOURCE_DIR}/engine/qcommon/q_math.cpp
    ${CMAKE_SOURCE_DIR}/engine/qcommon/q_shared.cpp
    ui_atoms.cpp
    ui_gameinfo.cpp
    ui_main.cpp
    ui_players.cpp
    ui_shared.cpp
    ui_syscalls.cpp
    ui_util.cpp)

if ( WIN32 )
    list(APPEND UI_SOURCES ui.def)
endif()

source_group(TREE "${CMAKE_SOURCE_DIR}" FILES ${UI_SOURCES})

add_library(ui SHARED ${UI_SOURCES})
set_target_properties(ui PROPERTIES OUTPUT_NAME uix86)
set_target_properties(ui PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:>${CMAKE_SOURCE_DIR}/main)
set_target_properties(ui PROPERTIES FOLDER "dlls")
if ( NOT WIN32 )
    set_target_properties(ui PROPERTIES PREFIX "")
endif()
target_compile_definitions(ui PRIVATE _USRDLL UI_EXPORTS)
if ( MSVC )
    target_link_libraries(ui PRIVATE odbc32 odbccp32)
    target_link_options(ui PRIVATE /BASE:0x40000000 /MAP:${CMAKE_SOURCE_DIR}/main/uix86.map)
endif()
target_include_directories(ui PRIVATE "${CMAKE_SOURCE_DIR}/engine/qcommon" "${CMAKE_SOURCE_DIR}/engine/renderer" "${CMAKE_SOURCE_DIR}/engine/client")
configure_target(ui)
