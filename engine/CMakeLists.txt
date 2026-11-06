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
	qcommon/net_message.cpp
	qcommon/net_chan.cpp
	qcommon/q_shared.cpp
	sound/snd_adpcm.cpp
	sound/snd_dma.cpp
	sound/snd_mem.cpp
	sound/snd_mix.cpp
	sound/snd_wavelet.cpp
	sound/snd_openal.cpp
	server/sv_bot.cpp
	server/sv_ccmds.cpp
	server/sv_client.cpp
	server/sv_game.cpp
	server/sv_init.cpp
	server/sv_main.cpp
	server/sv_net_chan.cpp
	server/sv_snapshot.cpp
	server/sv_world.cpp
	sys/sys_input.cpp
	sys/sys_main.cpp
	sys/sys_net.cpp)

if ( WIN32 )
	list(APPEND ENGINE_SOURCES sys/win32/win_syscon.cpp sys/win32/rc/rtcw.rc)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ENGINE_SOURCES})

add_library(miniz STATIC
	${CMAKE_SOURCE_DIR}/external/miniz/miniz.c)
target_include_directories(miniz PUBLIC
	${CMAKE_SOURCE_DIR}/external/miniz)
configure_target(miniz)

add_library(minizip STATIC
	${CMAKE_SOURCE_DIR}/external/minizip/unzip.cpp
	${CMAKE_SOURCE_DIR}/external/minizip/ioapi.c)
target_include_directories(minizip PUBLIC
	${CMAKE_SOURCE_DIR}/external/minizip
	${CMAKE_SOURCE_DIR}/external/miniz)
target_link_libraries(minizip PUBLIC miniz)
configure_target(minizip)

if ( WIN32 )
	add_executable(wolf WIN32 ${ENGINE_SOURCES})
else()
	add_executable(wolf ${ENGINE_SOURCES})
endif()

# Keep the engine translation units on the common idlib precompiled header.
# This header already contains the platform, idlib, and q_shared definitions
# shared by the engine and its renderer/library targets.
target_precompile_headers(wolf PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/precompiled.h")

set_target_properties(wolf PROPERTIES FOLDER "exes")
set_target_properties(wolf PROPERTIES OUTPUT_NAME WolfSP)
target_compile_definitions(wolf PRIVATE __USEA3D __A3D_GEOM CINTERFACE)

if ( WIN32 )
	target_link_libraries(wolf PRIVATE renderer botlib Splines idlib minizip SDL3::SDL3 OpenAL::OpenAL ws2_32 winmm opengl32)
	target_link_options(wolf PRIVATE /STACK:8388608 /MAP:${CMAKE_SOURCE_DIR}/bin/WolfSP.map /NXCOMPAT:NO)

	add_custom_command(TARGET wolf POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/external/SDL3/lib/x86/SDL3.dll" "$<TARGET_FILE_DIR:wolf>"
		COMMENT "Copying SDL3.dll next to WolfSP")

	add_custom_command(TARGET wolf POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/external/openal-soft/bin/Win32/soft_oal.dll" "$<TARGET_FILE_DIR:wolf>/OpenAL32.dll"
		COMMENT "Copying soft_oal.dll as OpenAL32.dll next to WolfSP")
else()
	target_link_libraries(wolf PRIVATE renderer botlib Splines idlib minizip SDL3::SDL3 OpenAL::OpenAL ${CMAKE_DL_LIBS})
endif()

target_include_directories(wolf PRIVATE
	${CMAKE_SOURCE_DIR}/engine/qcommon
	${CMAKE_SOURCE_DIR}/gamedll/game
	${CMAKE_SOURCE_DIR}/gamedll/ui
	${CMAKE_SOURCE_DIR}/gamedll/cgame
)

configure_target(wolf)

if ( HARD_LINKED_MODULES )
	if ( NOT APPLE )
		set_target_properties(wolf PROPERTIES ENABLE_EXPORTS ON)
	endif()
	if ( MSVC )
		target_compile_definitions(wolf PRIVATE ENGINE_EXPORTS)
		foreach( module game cgame ui )
			add_dependencies( ${module} wolf )
			target_link_libraries( ${module} PRIVATE "${CMAKE_BINARY_DIR}/lib/$<CONFIG>/WolfSP.lib" )
		endforeach()
	endif()
endif()

add_dependencies(wolf game cgame ui)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT wolf)
