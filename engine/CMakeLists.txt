add_subdirectory(splines)
add_subdirectory(renderer)

set(ENGINE_SOURCES
    client/cl_cgame.cpp
    client/cl_cin.cpp
    client/cl_console.cpp
    client/cl_input.cpp
    client/cl_keys.cpp
    client/cl_main.cpp
    client/cl_net_chan.cpp
    client/cl_parse.cpp
    client/cl_scrn.cpp
    client/cl_ui.cpp
    cm/cm_load.cpp
    cm/cm_patch.cpp
    cm/cm_polylib.cpp
    cm/cm_test.cpp
    cm/cm_trace.cpp
    qcommon/cmd.cpp
    qcommon/common.cpp
    qcommon/cvar.cpp
    qcommon/files.cpp
    qcommon/huffman.cpp
    qcommon/md4.cpp
    qcommon/msg.cpp
    qcommon/net_chan.cpp
    qcommon/q_math.cpp
    qcommon/q_shared.cpp
    sound/snd_adpcm.cpp
    sound/snd_dma.cpp
    sound/snd_mem.cpp
    sound/snd_mix.cpp
    sound/snd_wavelet.cpp
    server/sv_bot.cpp
    server/sv_ccmds.cpp
    server/sv_client.cpp
    server/sv_game.cpp
    server/sv_init.cpp
    server/sv_main.cpp
    server/sv_net_chan.cpp
    server/sv_snapshot.cpp
    server/sv_world.cpp
    qcommon/unzip.cpp
    sys/win32/win_input.cpp
    sys/win32/win_main.cpp
    sys/win32/win_net.cpp
    sys/win32/win_shared.cpp
    sys/win32/win_snd.cpp
    sys/win32/win_syscon.cpp
    sys/win32/win_wndproc.cpp
    sys/win32/winquake.rc)
	
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ENGINE_SOURCES})

add_executable(wolf WIN32 ${ENGINE_SOURCES})
set_target_properties(wolf PROPERTIES FOLDER "exes")
set_target_properties(wolf PROPERTIES OUTPUT_NAME WolfSP)
target_compile_definitions(wolf PRIVATE __USEA3D __A3D_GEOM CINTERFACE)
target_link_libraries(wolf PRIVATE renderer botlib Splines winmm wsock32 opengl32)

target_include_directories(wolf PRIVATE
	${CMAKE_SOURCE_DIR}/engine/qcommon
	${CMAKE_SOURCE_DIR}/gamedll/game
	${CMAKE_SOURCE_DIR}/gamedll/ui
	${CMAKE_SOURCE_DIR}/gamedll/cgame
)

target_link_options(wolf PRIVATE /STACK:8388608 /MAP:${CMAKE_SOURCE_DIR}/WolfSP.map /NXCOMPAT:NO)
configure_target(wolf)

add_dependencies(wolf game cgame ui)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT wolf)