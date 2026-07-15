# DATA — Il2Cpp dump reference material

This folder holds Il2Cpp "dummy.cs" style dumps (from Il2CppDumper) for specific
Mobile Legends: Bang Bang builds. They are **reference material only** — not
compiled, not shipped in the APK. Use them to re-derive class/struct layouts
and offsets whenever the hooks in `app/src/main/jni/feature/*.h` go stale
after a game update.

## Files

- `com.mobile.legends_2.1.88.12027_1784136322181.cs` — full Il2Cpp dump for
  MLBB `2.1.88.12027`.

Naming convention: `<package>_<game version>_<upload timestamp>.cs`.

## How to use a dump

1. Identify which classes/structs are referenced in
   `app/src/main/jni/feature/*.h` and `app/src/main/jni/unity/Il2Cpp.h`
   (e.g. `GameClass`, `RoomInfo`, Il2Cpp string/array helpers).
2. `grep` the dump for those class names to find their current field order
   and types for this game version.
3. Recompute field offsets from the dump (Il2CppDumper lists fields in
   declaration order; offsets follow standard struct packing/alignment for
   the target ABI — check both armeabi-v7a and arm64-v8a since pointer size
   differs).
4. Update the offsets/struct definitions in `feature/*.h` accordingly, and
   note in the commit message which game version the update targets.

## Status

Offset re-derivation against this dump has **not started yet** — this file
and the dump were added first so the workflow is documented before the
actual offset work begins.
