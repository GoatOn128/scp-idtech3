# Milestone 4 - botlib as a separate DLL

Finished: Wednesday, November 4, 8:31 PM

What changed:

- botlib now builds as a shared library instead of a static archive: the exe loads it at startup through GetBotLibAPI, the only exported symbol. gamedll/game/botlib.h declares GetBotLibAPI extern "C" behind a __cplusplus guard so the engine's sv_bot.cpp and the library agree on the symbol name.
- The DLL carries its own copies of the engine qcommon code, exactly like the game module: gamedll/botlib/CMakeLists.txt compiles engine/qcommon/q_math.cpp and q_shared.cpp into the library and adds the qcommon include dir.
- Com_Error and Com_Printf are defined inside botlib (be_interface.cpp) and routed through the bot import table (botimport.Print, PRT_FATAL and PRT_MESSAGE), mirroring the game module's own copies in g_main.cpp.
- Windows exports GetBotLibAPI through the new gamedll/botlib/botlib.def; MSVC also links winmm into botlib because Sys_MilliSeconds uses timeGetTime, which the exe's link used to cover when botlib was static.
- external/CMakeLists.txt builds detour with POSITION_INDEPENDENT_CODE ON so the static detour can be linked into the shared botlib on Linux.

Build health at merge: all 6 CI jobs green, 0 errors on every platform; Windows Debug and Release both at 0 warnings (MSVC /W4). No new warning kinds were introduced; the remaining Linux and macOS warnings are the pre-existing cleanup backlog outside the m4 scope.
