# Milestone 2 - OpenAL Soft audio

Finished: Wednesday, November 4, 8:06 PM

What changed:

- OpenAL Soft replaces the SDL3 audio backend: engine/sys/sys_snd.cpp is gone, the SNDDMA_* backend now lives in engine/sys/openal/snd_openal.cpp, and the engine's software mixer is untouched.
- The DMA ring is preserved: same 64KB stereo 16-bit ring (dma.samples 32768 interleaved samples, speed from s_khz), so s_mixAhead and the rest of snd_dma keep working against the usual SNDDMA interface.
- Playback runs through a pool of 16 OpenAL buffers (2048 interleaved samples each) queued on one stereo source: SNDDMA_Submit copies painted ring chunks into free buffers and splits them at ring wraps, SNDDMA_GetDMAPos derives the position from what OpenAL finished plus AL_SAMPLE_OFFSET, and the source restarts after underruns.
- OpenAL Soft (LGPL) is vendored in external/openal-soft: Windows links the SDK import lib and ships soft_oal.dll renamed to OpenAL32.dll next to the exe, Linux uses libopenal-dev, and CI builds the 1.23.1 tarball on macOS with CMAKE_POLICY_VERSION_MINIMUM=3.5 because the macos-14 runners ship CMake 4.
- sys_main.cpp no longer passes SDL_INIT_AUDIO; SNDDMA_Activate stays a no-op for the sys_local.h contract.

Build health at merge: all 6 CI jobs green, 0 errors on every platform; Windows Debug and Release both at 0 warnings (MSVC /W4). The new backend compiles clean on all platforms; the remaining Linux and macOS warnings are the pre-existing cleanup backlog outside the m2 scope.
