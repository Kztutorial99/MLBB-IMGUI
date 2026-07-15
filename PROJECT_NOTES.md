# Project notes (for future contributors / agents)

## ABI support
- arm64-v8a build support was added alongside the original armeabi-v7a:
  - `app/src/main/jni/Application.mk` → `APP_ABI := armeabi-v7a arm64-v8a`
  - `app/build.gradle` → `abiFilters 'armeabi-v7a', 'arm64-v8a'`
- The prebuilt `dobby/libraries/arm64-v8a/libdobby.a` already existed in the
  repo, and the C++ hook/syscall code (`dobby.h`, `unity/Tools.cpp`) already
  branches on `__aarch64__`. No source changes were needed for arm64 — only
  the build config above was gating it off.

## Offset research workflow
- `DATA/` holds Il2Cpp "dummy.cs" dumps (from Il2CppDumper) per MLBB game
  version — reference only, not compiled/shipped. See `DATA/README.md` for
  naming convention and the step-by-step process to re-derive offsets in
  `app/src/main/jni/feature/*.h` from a dump when the game updates and the
  hardcoded offsets go stale.
- Current dump on hand: `DATA/com.mobile.legends_2.1.88.12027_1784136322181.cs`
  (MLBB `2.1.88.12027`). Offset re-derivation against it has not started yet.
