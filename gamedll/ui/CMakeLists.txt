set(UI_SOURCES
    ../game/bg_lib.cpp
    ../game/bg_misc.cpp
    ${CMAKE_SOURCE_DIR}/qcommon/q_math.cpp
    ${CMAKE_SOURCE_DIR}/qcommon/q_shared.cpp
    ui/ui_atoms.cpp
    ui/ui_gameinfo.cpp
    ui/ui_main.cpp
    ui/ui_players.cpp
    ui/ui_shared.cpp
    ui/ui_syscalls.cpp
    ui/ui_util.cpp
    ui/ui.def)
	
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${UI_SOURCES})

add_library(ui SHARED ${UI_SOURCES})
set_target_properties(ui PROPERTIES OUTPUT_NAME uix86)
set_target_properties(ui PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:>${CMAKE_SOURCE_DIR}/main)
set_target_properties(ui PROPERTIES FOLDER "dlls")
target_compile_definitions(ui PRIVATE _USRDLL UI_EXPORTS)
target_link_libraries(ui PRIVATE odbc32 odbccp32)
target_link_options(ui PRIVATE /BASE:0x40000000 /MAP:${CMAKE_SOURCE_DIR}/main/uix86.map)
configure_target(ui)