set(RECAST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/recast")
set(RECAST_SOURCES
    recast/Recast/Source/Recast.cpp
    recast/Recast/Source/RecastAlloc.cpp
    recast/Recast/Source/RecastArea.cpp
    recast/Recast/Source/RecastAssert.cpp
    recast/Recast/Source/RecastContour.cpp
    recast/Recast/Source/RecastFilter.cpp
    recast/Recast/Source/RecastLayers.cpp
    recast/Recast/Source/RecastMesh.cpp
    recast/Recast/Source/RecastMeshDetail.cpp
    recast/Recast/Source/RecastRasterization.cpp
    recast/Recast/Source/RecastRegion.cpp)

set(DETOUR_SOURCES
    recast/Detour/Source/DetourAlloc.cpp
    recast/Detour/Source/DetourAssert.cpp
    recast/Detour/Source/DetourCommon.cpp
    recast/Detour/Source/DetourNavMesh.cpp
    recast/Detour/Source/DetourNavMeshBuilder.cpp
    recast/Detour/Source/DetourNavMeshQuery.cpp
    recast/Detour/Source/DetourNode.cpp)

set(GLAD_SOURCES
    glad/src/gl.c)

add_library(recast STATIC ${RECAST_SOURCES})
target_include_directories(recast PUBLIC "${RECAST_ROOT}/Recast/Include")
configure_target(recast)

add_library(detour STATIC ${DETOUR_SOURCES})
target_include_directories(detour PUBLIC "${RECAST_ROOT}/Detour/Include")
configure_target(detour)

# DetourNavMesh.cpp trips C4127 in its table-driven code; silence it like any other third-party library.
target_compile_options(detour PRIVATE /wd4127)

# botlib is a shared library now and links detour into it, so the detour
# objects must be position independent code on Linux
set_target_properties(detour PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(glad STATIC ${GLAD_SOURCES})
target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/include")
configure_target(glad)

# SDL3: use the vendored prebuilt binaries on Windows, the system package
# elsewhere. GLOBAL is required: imported targets are directory-scoped and
# engine/renderer link against SDL3::SDL3 from sibling directories.
if ( WIN32 )
    add_library(SDL3_vendored SHARED IMPORTED GLOBAL)
    set_target_properties(SDL3_vendored PROPERTIES
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/SDL3/lib/x86/SDL3.lib"
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/SDL3/lib/x86/SDL3.dll"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/SDL3/include")
    add_library(SDL3::SDL3 ALIAS SDL3_vendored)
else()
    find_package(SDL3 REQUIRED GLOBAL)
endif()

# OpenAL Soft: vendored Windows SDK binaries on Windows, system package
# elsewhere. The DLL is shipped next to the exe renamed to OpenAL32.dll
# because that is the name the import library talks to. GLOBAL is
# required: engine links OpenAL::OpenAL from a sibling directory.
if ( WIN32 )
    add_library(OpenAL_vendored SHARED IMPORTED GLOBAL)
    set_target_properties(OpenAL_vendored PROPERTIES
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/openal-soft/libs/Win32/OpenAL32.lib"
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/openal-soft/bin/Win32/soft_oal.dll"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/openal-soft/include")
    add_library(OpenAL::OpenAL ALIAS OpenAL_vendored)
else()
    find_package(OpenAL REQUIRED GLOBAL)
endif()
