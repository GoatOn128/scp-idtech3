set(RENDERER_SOURCES
    tr_animation.cpp
    tr_backend.cpp
    tr_renderprog.cpp
    tr_decals.cpp
    tr_vfx.cpp
    tr_bsp.cpp
    tr_cmds.cpp
    tr_cmesh.cpp
    tr_curve.cpp
    tr_flares.cpp
    tr_font.cpp
    tr_image.cpp
    tr_init.cpp
    tr_light.cpp
    tr_main.cpp
    tr_marks.cpp
    tr_mesh.cpp
    tr_model.cpp
    tr_noise.cpp
    tr_scene.cpp
    tr_shade.cpp
    tr_shade_calc.cpp
    tr_shader.cpp
    tr_shadows.cpp
    tr_sky.cpp
    tr_surface.cpp
    tr_world.cpp
    ../sys/win32/win_gamma.cpp
    ../sys/win32/win_glimp.cpp)

add_library(renderer STATIC ${RENDERER_SOURCES})
set_target_properties(renderer PROPERTIES FOLDER "libs")
target_compile_definitions(renderer PRIVATE _LIB __USEA3D __A3D_GEOM)
target_include_directories(renderer PRIVATE
    "${CMAKE_SOURCE_DIR}/external/stb"
    "${CMAKE_SOURCE_DIR}/external/glad/include")
target_link_libraries(renderer PUBLIC glad)
configure_target(renderer)