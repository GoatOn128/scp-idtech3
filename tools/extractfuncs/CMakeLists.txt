set(EXTRACTFUNCS_SOURCES
    extractfuncs.cpp
    l_log.cpp
    l_memory.cpp
    l_precomp.cpp
    l_script.cpp)

add_executable(extractfuncs ${EXTRACTFUNCS_SOURCES})
set_target_properties(extractfuncs PROPERTIES FOLDER "exes")
target_compile_definitions(extractfuncs PRIVATE _CONSOLE SCREWUP)
target_compile_options(extractfuncs PRIVATE /Od)
configure_target(extractfuncs)

add_custom_target(genFuncTable
    COMMAND $<TARGET_FILE:extractfuncs> "*.cpp" "ai/*.cpp" "ai/fsm/*.cpp"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/gamedll/game"
    DEPENDS extractfuncs
    COMMENT "Building the game function table"
    VERBATIM
)