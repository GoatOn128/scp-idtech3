set(GAME_SOURCES
    ai/fsm/ai_ability.cpp
    ai/ai_system.cpp
    ai/ai_characters.cpp
    ai/ai_spawn.cpp
    ai/ai_debug.cpp
    ai/ai_events.cpp
    ai/fsm/ai_state_machine.cpp
    ai/fsm/ai_state_legacy.cpp
    ai/ai_awareness.cpp
    ai/ai_targeting.cpp
    ai/ai_perception.cpp
    ai/ai_weapons.cpp
    ai/fsm/ai_combat.cpp
    ai/fsm/ai_navigation.cpp
    ai/fsm/ai_danger.cpp
    ai/fsm/ai_monster_zombie.cpp
    ai/fsm/ai_monster_loper.cpp
    ai/fsm/ai_monster_stimsoldier.cpp
    ai/fsm/ai_monster_blackguard.cpp
    ai/fsm/ai_monster_reject.cpp
    ai/fsm/ai_monster_warzombie.cpp
    ai/fsm/ai_boss_helga.cpp
    ai/fsm/ai_boss_heinrich.cpp
    ai/ai_movement.cpp
    ai/fsm/ai_behavior_doors.cpp
    ai/fsm/ai_behavior_idle.cpp
    ai/fsm/ai_behavior_investigate.cpp
    ai/fsm/ai_behavior_pursuit.cpp
    ai/fsm/ai_behavior_evasion.cpp
    ai/fsm/ai_behavior_cover.cpp
    ai/fsm/ai_behavior_grenades.cpp
    ai/fsm/ai_behavior_mounted.cpp
    ai/ai_script.cpp
    ai/ai_script_actions_combat.cpp
    ai/ai_script_actions_movement.cpp
    ai/ai_script_actions_mission.cpp
    ai/ai_script_actions_animation.cpp
    ai/ai_script_entities.cpp
    ai/ai_input.cpp
    ai/ai_actor.cpp
    ai/ai_scheduler.cpp
    bg_animation.cpp
    bg_lib.cpp
    bg_misc.cpp
    bg_pmove.cpp
    bg_slidemove.cpp
    g_active.cpp
    g_alarm.cpp
    g_client.cpp
    g_cmds.cpp
    g_combat.cpp
    g_items.cpp
    g_main.cpp
    g_mem.cpp
    g_misc.cpp
    g_missile.cpp
    g_mover.cpp
    g_props.cpp
    g_save.cpp
    g_script.cpp
    g_script_actions.cpp
    g_session.cpp
    g_spawn.cpp
    g_svcmds.cpp
    g_syscalls.cpp
    g_target.cpp
    g_team.cpp
    g_tramcar.cpp
    g_trigger.cpp
    g_utils.cpp
    g_weapon.cpp
    ${CMAKE_SOURCE_DIR}/engine/qcommon/q_math.cpp
    ${CMAKE_SOURCE_DIR}/engine/qcommon/q_shared.cpp)

if ( WIN32 )
    list(APPEND GAME_SOURCES game.def)
endif()

source_group(TREE "${CMAKE_SOURCE_DIR}" FILES ${GAME_SOURCES})

add_library(game SHARED ${GAME_SOURCES})
set_target_properties(game PROPERTIES OUTPUT_NAME qagamex86)
set_target_properties(game PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:>${CMAKE_SOURCE_DIR}/main)
set_target_properties(game PROPERTIES FOLDER "dlls")
if ( NOT WIN32 )
    set_target_properties(game PROPERTIES PREFIX "")
endif()
target_compile_definitions(game PRIVATE GAMEDLL $<$<CONFIG:Debug>:BUILDING_REF_GL>)
if ( MSVC )
    target_link_libraries(game PRIVATE winmm)
    target_link_options(game PRIVATE /BASE:0x20000000 /MAP:${CMAKE_SOURCE_DIR}/main/qagamex86.map)
endif()
if ( WIN32 )
    add_dependencies(game genFuncTable)
endif()
target_include_directories(game PRIVATE "${CMAKE_SOURCE_DIR}/engine/qcommon")
configure_target(game)
