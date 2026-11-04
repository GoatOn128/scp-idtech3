# Milestone 0 - project files and CI

Finished: Tuesday, November 3, 11:34 PM

What changed:

- New README.md describing the project, layout, build and third-party set.
- ci-windows.yml rewritten from the stale SCons copy to CMake: Win32 matrix over Debug and Release on windows-latest, digest via the existing publish pipeline (unchanged ci-publish.yml, assemble.py).
- digest.py error regex extended for the CMake/MSVC world (CMake Error, error LNK, error MSB, Build FAILED) and the failure-context section now keys on CMake/MSVC targets instead of scons.
- VS Code config rewritten for this tree: include paths for engine/, gamedll/ and external/, tasks driving the CMake presets, launch config for bin/WolfSP.exe.
- Agent docs (CLAUDE.md, .github/copilot-instructions.md, .claude/agents, .github/agents) rewritten for the CMake project rules: digest-first CI loop, zero-warning bar, [skip ci] convention, feature branch + PR + squash workflow, external/ and references/ untouchable.
- .gitignore for the CMake output directories.
