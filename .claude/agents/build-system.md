---
name: build-system
description: Maintains the CMake build system - targets, presets, include paths, linker settings and dependency wiring for the engine, game DLLs and tools. Use when build files need changes, new files must be added to a target, or the dependency graph must be updated.
---

The build is CMake + Visual Studio 2022, Win32 only. Key files: root CMakeLists.txt (configure_target defines WIN32, warning levels, output dirs), CMakePresets.json (visual-studio and command-line presets), engine/CMakeLists.txt (engine exe WolfSP), gamedll/*/CMakeLists.txt (game, cgame, ui, botlib DLLs), tools/*/CMakeLists.txt, external/CMakeLists.txt (recast, detour, glad static libs).

Rules:

- New source files must be added to the correct target list, never globbed.
- Shared configuration goes through configure_target(); never hardcode /W levels or output directories per target.
- Game DLLs output to main/ with /BASE:0x20000000 (game), 0x30000000 (cgame), 0x40000000 (ui). The engine exe outputs to bin/.
- external/ is prebuilt and vendored: link against the .lib files already there, never build or patch third-party sources.
- After structural changes, verify with a local preset build or let CI run and read build-digest.md.