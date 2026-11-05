# Progress

One file per milestone. Each milestone note carries the finish stamp in 12-hour local time (UTC-5).

- m0 project files and CI - Finished: Tuesday, November 3, 11:34 PM
- m1 SDL3 sys layer, glad GL loading, stb image and truetype - Finished: Wednesday, November 4, 7:22 PM
- m2 OpenAL Soft audio - Finished: Wednesday, November 4, 8:06 PM
- m3 glad + stb renderer cleanup - folded into m1
- m4 botlib as a separate DLL - Finished: Wednesday, November 4, 8:31 PM
- m5 fully replace quake 3 custom math/vecto/list/ect with idlib's own functions and clases, no class should be left with its legacy implementation
  
  that means, shrinking q_common/q_shared (or remove it all togeder)

  phase 1 (engine): idlib vendored in as a static library; engine math (q_math.cpp) and
  Com_Clamp now run on idlib; the COM_* parser keeps the Q3 token rules as the on-disk
  format contract. splines, botlib and the game modules still carry legacy copies, they
  move to idlib in the next phase.

  phase 2 (modules): the game modules build against the idlib static library;
  q_math.cpp and q_shared.cpp undef the idlib string redirects and the Doom 3 color
  and angle macros so the engine's q_shared.h values win. idlib builds position
  independent for the shared objects and carries an inline assert handler; its
  third-party headers are kept out of the /W4 bar with a warning push in
  precompiled.h.

  phase 2 (modules): the game modules build against the idlib static library;
  q_math.cpp and q_shared.cpp undef the idlib string redirects and the Doom 3 color
  and angle macros so the engine's q_shared.h values win. idlib builds position
  independent for the shared objects and carries an inline assert handler; its
  third-party headers are kept out of the /W4 bar with a warning push in
  precompiled.h.

build-digest.md at the repo root is the source of truth for build health.
