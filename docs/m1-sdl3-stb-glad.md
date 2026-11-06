# Milestone 1 - SDL3 system layer, glad, stb

Finished: Wednesday, November 4, 7:22 PM

What changed:

- stb replaces the image and font codecs: tr_image.cpp loads JPEG/TGA/BMP through stb_image (jpeg-6 and the hand-written loaders are gone), and tr_font.cpp renders TrueType at runtime with stb_truetype while keeping the prerendered fonts/fontImage_*.dat path and r_saveFontData.
- glad replaces the qgl function-pointer loading: win_qgl.cpp and qgl_linked.h are gone, the renderer loads entry points through the vendored glad loader context, the dead GL extension and per-chipset GPU hacks are removed, and Mesa-based drivers are only noted in the log.
- SDL3 replaces the portable parts of sys/win32: new engine/sys/sdl layer (sys_main, sys_input, sys_net, sys_snd, sys_glimp); win_main, win_wndproc, win_input, win_net, win_snd, win_shared, win_glimp and win_gamma removed; sys/win32 keeps only the console window and resources. The SDL3 backend runs without hardware gamma (SDL3 dropped the ramp API) and without the SMP render thread.
- Cross-platform build and CI: 6-job CI matrix (linux-x86-64, macos-arm64, windows-x86, each Debug and Release), SDL3 vendored prebuilt on Windows and a system package elsewhere, POSIX name aliases and C++ min/max templates in q_shared.h, portable plain-C Com_Memcpy/Com_Memset on non-MSVC builds.
- Portability and warning fixes across the engine and game modules: 64-bit pointer casts through intptr_t/uintptr_t, C++ keyword parameter renames, and MSVC /W4 cleanups, with Windows Debug and Release both at 0 warnings.

Build health at merge: all 6 CI jobs green. The documented zero-warning bar (MSVC /W4) is met on Windows; Linux and macOS build with 0 errors, with their warning cleanup outside the m1 scope.
