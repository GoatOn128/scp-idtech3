set(SPLINES_SOURCES
    splines.cpp)

add_library(Splines STATIC ${SPLINES_SOURCES})
target_include_directories(Splines PRIVATE "${CMAKE_SOURCE_DIR}/external/glad/include")
set_target_properties(Splines PROPERTIES FOLDER "libs")
target_compile_definitions(Splines PRIVATE _LIB)
target_link_libraries(Splines PRIVATE idlib)
configure_target(Splines)
