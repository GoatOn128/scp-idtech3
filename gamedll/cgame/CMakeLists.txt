set(CGAME_SOURCES
    cg_fx.cpp
    FxSystem.cpp
    FxUtil.cpp
    FxPrimitives.cpp
    FxScheduler.cpp
    FxTemplate.cpp
    genericparser2.cpp
    ../game/bg_animation.cpp
    ../game/bg_lib.cpp
    ../game/bg_misc.cpp
    ../game/bg_pmove.cpp
    ../game/bg_slidemove.cpp
    cg_consolecmds.cpp
    cg_draw.cpp
    cg_drawtools.cpp
    cg_effects.cpp
    cg_ents.cpp
    cg_event.cpp
    cg_fx_flame.cpp
    cg_info.cpp
    cg_fx_entities.cpp
    cg_main.cpp
    cg_fx_marks.cpp
    cg_newDraw.cpp
    cg_fx_particles.cpp
    cg_players.cpp
    cg_playerstate.cpp
    cg_predict.cpp
    cg_scoreboard.cpp
    cg_servercmds.cpp
    cg_snapshot.cpp
    cg_sound.cpp
    cg_syscalls.cpp
    cg_fx_trails.cpp
    cg_view.cpp
    cg_weapons.cpp
    ${CMAKE_SOURCE_DIR}/engine/qcommon/q_math.cpp
    ${CMAKE_SOURCE_DIR}/engine/qcommon/q_shared.cpp
    ../ui/ui_shared.cpp
    cgame.def)

source_group(TREE "${CMAKE_SOURCE_DIR}" FILES ${CGAME_SOURCES})

add_library(cgame SHARED ${CGAME_SOURCES})
set_target_properties(cgame PROPERTIES OUTPUT_NAME cgamex86)
set_target_properties(cgame PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:>${CMAKE_SOURCE_DIR}/main)
set_target_properties(cgame PROPERTIES FOLDER "dlls")
target_compile_definitions(cgame PRIVATE CGAMEDLL)
target_link_libraries(cgame PRIVATE odbc32 odbccp32)
target_link_options(cgame PRIVATE /BASE:0x30000000 /MAP:${CMAKE_SOURCE_DIR}/main/cgamex86.map)
target_include_directories(cgame PRIVATE "${CMAKE_SOURCE_DIR}/engine/qcommon")
configure_target(cgame)
