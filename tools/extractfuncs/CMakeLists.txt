set(EXTRACTFUNCS_SOURCES
    extractfuncs.cpp)

add_executable(extractfuncs ${EXTRACTFUNCS_SOURCES})
set_target_properties(extractfuncs PROPERTIES FOLDER "exes")
target_compile_definitions(extractfuncs PRIVATE _CONSOLE SCREWUP)
target_compile_options(extractfuncs PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/Od>)
target_link_libraries(extractfuncs PRIVATE idlib)
target_compile_features(extractfuncs PRIVATE cxx_std_17)
configure_target(extractfuncs)
target_precompile_headers(extractfuncs PRIVATE "${CMAKE_SOURCE_DIR}/tools/precompiled.h")

add_custom_target(genFuncTable
    COMMAND $<TARGET_FILE:extractfuncs> "*.cpp" "ai/*.cpp" "ai/fsm/*.cpp"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/gamedll/game"
    DEPENDS extractfuncs
    COMMENT "Building the game function table"
    VERBATIM
)