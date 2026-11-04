# SCP-idTech3

A modernized single-player port of the Return to Castle Wolfenstein GPL engine (id Tech 3). The tree builds with CMake and Visual Studio 2022 (Win32), uses SDL3 for windowing/input/timing, OpenAL Soft for audio, glad as the OpenGL loader, and stb headers for image and font work. Third-party code is prebuilt or vendored under external/.

## Layout

- engine/ - the engine executable (client, server, qcommon, renderer, sound, sys).
- gamedll/ - the game modules built as DLLs: game (qagamex86), cgame (cgamex86), ui (uix86), botlib (botlibx86).
- tools/ - offline tools: bspc (navmesh builder), extractfuncs (game function table generator).
- external/ - third-party code and prebuilt Win32 libraries: SDL3, OpenAL Soft, glad, stb, recast/detour.
- references/ - reference code that is not part of the build.
- docs/ - progress notes, one file per milestone, each stamped with the finish time.
- bin/ - build output for the engine executable (created by the build).
- main/ - build output for the game DLLs (created by the build).

## Building

Requirements: Visual Studio 2022 with C++ tools (x86/x64 toolset), CMake 3.25 or newer.

    cmake --preset visual-studio
    cmake --build --preset visual-studio-release

or from a developer command prompt:

    cmake -S . -B out -G "Visual Studio 17 2022" -A Win32
    cmake --build out --config Release

The engine lands in bin/WolfSP.exe, the game DLLs in main/. SDL3.dll and OpenAL32.dll are copied to bin/ after the build. Copy the game data into main/ to run.

## Continuous integration

build-windows (ci-windows.yml) builds Debug and Release on windows-latest and publishes a per-commit digest: build-digest.md plus build-windows.log at the repo root for main, on a ci-logs/ ref for feature branches. Commits whose message contains [skip ci] do not trigger it.

## Third-party and licensing

- Engine and game code: Return to Castle Wolfenstein single-player source, GPL-2.0+ as released by id Software (this repo is a derivative work under the same terms).
- SDL3 (external/SDL3): zlib license.
- OpenAL Soft (external/openal-soft): LGPL-2.1+, linked dynamically.
- glad (external/glad): MIT/CC0 generated loader.
- stb (external/stb): public domain / MIT.
- recast/detour (external/recast): zlib license.

See docs/ for milestone progress notes.
