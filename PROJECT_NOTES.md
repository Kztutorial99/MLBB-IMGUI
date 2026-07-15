# Project notes (for future contributors / agents)

## ABI support
- arm64-v8a build support was added alongside the original armeabi-v7a:
  - `app/src/main/jni/Application.mk` → `APP_ABI := armeabi-v7a arm64-v8a`
  - `app/build.gradle` → `abiFilters 'armeabi-v7a', 'arm64-v8a'`
- The prebuilt `dobby/libraries/arm64-v8a/libdobby.a` already existed.
  C++ hook/syscall code (`dobby.h`, `unity/Tools.cpp`) already branches on
  `__aarch64__`. No source changes were needed — only build config.

## MLBB 2.1.88.12027 — offset update (done)

### What changed in this game version vs. previous source

| File | Change |
|---|---|
| `feature/GameClass.h` | `Battle.EntityBase` no longer registered in Assembly-CSharp metadata — all `EntityBase_*` field lookups moved to `ShowEntity` (Show layer). `_Position` field renamed to `m_EntityObjPos`. `TryUseSkill` hook target moved from `Battle.ShowUnitAIComp` to `ShowPlayer`. `LogicPlayer_m_KillWildTimes` now sourced from `Battle.LogicFighter`. |
| `feature/AutoAim.h` | Hook signature updated for new `ShowPlayer.TryUseSkill` 9-param overload: return type `int` (was `void`), `bAuto` removed, `isInFirstDragRange` / `bIgnoreQueue` / `dragTime` added. All `orig_TryUseSkill` call-sites updated. Safe default added for `iCalculateDamage` when `_logicFighter` is null. |
| `feature/RoomInfo.h` | `lUid` and `m_uiID` types changed `int` → `uint64_t` (field is `System.UInt64` at offset `0x20` in `SystemData.RoomData`). |

### Fields confirmed still present in 2.1.88 dump (no change needed)
- `BattleManager`: `Instance`, `m_LocalPlayerShow` (0x50), `m_ShowPlayers` (0x78),
  `m_ShowMonsters` (0x80), `_RunBullets` (static)
- `ShowEntity`: `m_ID` (0x194), `m_Level` (0x198), `m_Hp` (0x1ac),
  `m_HpMax` (0x1b0), `m_bDeath` (0xcd), `m_bSameCampType` (0x2b1),
  `canSight` (0x352), `m_EntityObjPos` (0x310), `_logicFighter` (0x3c8)
- `SystemData.m_uiID` (0x340, UInt64)
- `BattleData.m_BattleBridge` (static)
- `LogicBattleManager.GetBattleState` method
- All `SystemData` UnlockSkin methods (GetHeroSkin, IsHaveSkin, etc.)

### Fields/classes that no longer exist
- `Battle.EntityBase` — removed from Assembly-CSharp; fields are now on `ShowEntity`
- `ShowUnitAIComp.TryUseSkill` — method moved to `ShowPlayer`; `ShowUnitAIComp`
  class itself moved to `Battle` namespace (hook for `Update` still valid there)
- `EntityBase._Position` field — renamed to `m_EntityObjPos` in `ShowEntity`
- `m_IsDeathProgress` — not in `ShowEntity`; substituted with `m_bDeadAnima` (0x291)

## Offset research workflow
- `DATA/` holds Il2Cpp "dummy.cs" dumps per MLBB game version — reference only,
  not compiled/shipped. See `DATA/README.md` for naming convention and process.
- Current dump: `DATA/com.mobile.legends_2.1.88.12027_1784136322181.cs`.
  Offset re-derivation against this dump is **complete** (see table above).
