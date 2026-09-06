set(BSPC_SOURCES
    bspc.cpp
    navmesh_builder.cpp
    navmesh_builder.h
    ${CMAKE_SOURCE_DIR}/engine/nav/wolf_nav.h)

add_executable(bspc ${BSPC_SOURCES})

target_include_directories(bspc PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/gamedll/game")
target_link_libraries(bspc PRIVATE recast detour)
target_compile_features(bspc PRIVATE cxx_std_17)
target_compile_definitions(bspc PRIVATE NOMINMAX)
configure_target(bspc)