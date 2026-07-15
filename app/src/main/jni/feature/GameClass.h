#pragma once

//Class LogicBattleManager
#define LogicBattleManager_GetBattleState (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "LogicBattleManager", "GetBattleState")

int GetBattleState(void* instance) {
	return reinterpret_cast<int(__fastcall *)(void*)>(LogicBattleManager_GetBattleState)(instance);
}

//Class SystemData
#define SystemData_m_uiID (uintptr_t) Il2CppGetStaticFieldOffset("Assembly-CSharp.dll","","SystemData","m_uiID")

#define SystemData_GetBattlePlayerInfo (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "SystemData", "GetBattlePlayerInfo")

//Class SystemData.RoomData
#define SystemData_RoomData_iCamp (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "SystemData/RoomData", "iCamp")
#define SystemData_RoomData__sName (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "SystemData/RoomData", "_sName")
#define SystemData_RoomData__steamSimpleName (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "SystemData/RoomData", "_steamSimpleName")
#define SystemData_RoomData__steamName (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "SystemData/RoomData", "_steamName")
#define SystemData_RoomData_uiRankLevel (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "SystemData/RoomData", "uiRankLevel")
#define SystemData_RoomData_iMythPoint (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "SystemData/RoomData", "iMythPoint")
#define SystemData_RoomData_heroid (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "SystemData/RoomData", "heroid")
#define SystemData_RoomData_summonSkillId (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "SystemData/RoomData", "summonSkillId")
#define SystemData_RoomData_lUid (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "SystemData/RoomData", "lUid")
#define SystemData_RoomData_uiZoneId (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "SystemData/RoomData", "uiZoneId")

//Class Camera
#define Camera_get_main (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Camera", "get_main")
#define Camera_WorldToScreenPoint (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Camera", "WorldToScreenPoint", 1)
#define Camera_get_fieldOfView (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Camera", "get_fieldOfView")
#define Camera_set_fieldOfView (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Camera", "set_fieldOfView", 1)

void *get_main() {
	return reinterpret_cast<void *(__fastcall *)()>(Camera_get_main)();
}

Vector3 WorldToScreen(Vector3 position) {
	return reinterpret_cast<Vector3(__fastcall *)(void *, Vector3)>(Camera_WorldToScreenPoint)(get_main(), position);
}

float get_fieldOfView() {
	return reinterpret_cast<float(__fastcall *)(void *)>(Camera_get_fieldOfView)(get_main());
}

void *set_fieldOfView(float value) {
	return reinterpret_cast<void *(__fastcall *)(void *, float)>(Camera_set_fieldOfView)(get_main(), value);
}

//Class Transform
#define Transform_get_position (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Transform", "get_position")

class Transform {
public:
    Vector3 get_position() {
        auto get_position = (Vector3 (*)(Transform *)) (Transform_get_position);
        return get_position(this);
    }
};

//Class BattleBridge
#define BattleBridge_bStartBattle (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "BattleBridge", "bStartBattle")

#define BattleBridge_SetMapEntityIconPos (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "BattleBridge", "SetMapEntityIconPos", 3)
#define BattleBridge_SetMapInvisibility (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "BattleBridge", "SetMapInvisibility", 3)
#define BattleBridge_SetBloodInvisibility (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "BattleBridge", "SetBloodInvisibility", 5)
#define BattleBridge_SynBloodAttr (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "BattleBridge", "SynBloodAttr", 2)
#define BattleBridge_UpdateBloodPos (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "BattleBridge", "UpdateBloodPos", 2)

//Class BattleManager
#define BattleManager_m_LocalPlayerShow (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "BattleManager", "m_LocalPlayerShow")
#define BattleManager_m_ShowPlayers (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "BattleManager", "m_ShowPlayers")
#define BattleManager_m_ShowMonsters (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "BattleManager", "m_ShowMonsters")
#define BattleManager__RunBullets (uintptr_t) Il2CppGetStaticFieldOffset("Assembly-CSharp.dll", "", "BattleManager", "_RunBullets")

#define BattleManager_Update (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "BattleManager", "Update")

// Class ShowEntity
// UPDATED v2.1.88: _Position renamed to m_EntityObjPos; all EntityBase_* now
// resolved from ShowEntity (EntityBase was refactored out in this version).
#define ShowEntity__Position (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "ShowEntity", "m_EntityObjPos")
#define ShowEntity_m_RoleName (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "ShowEntity", "m_RoleName")
#define ShowEntity__logicFighter (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "ShowEntity", "_logicFighter")

// Class ShowEntity — entity state fields
// (previously sourced from Battle.EntityBase which no longer exists as a
//  separate class in Assembly-CSharp metadata for this game version;
//  fields are now directly declared on ShowEntity)
#define EntityBase_m_bSameCampType (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "ShowEntity", "m_bSameCampType")
#define EntityBase_m_IsDeathProgress (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "ShowEntity", "m_bDeadAnima")
#define EntityBase_m_bDeath (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "ShowEntity", "m_bDeath")
#define EntityBase_canSight (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "ShowEntity", "canSight")
#define EntityBase_m_Hp (uintptr_t)  Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "ShowEntity", "m_Hp")
#define EntityBase_m_HpMax (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "ShowEntity", "m_HpMax")
#define EntityBase_m_ID (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "ShowEntity", "m_ID")
#define EntityBase_m_uGuid (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "ShowEntity", "m_uGuid")
#define EntityBase_m_Level (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "ShowEntity", "m_Level")

//Class ShowPlayer
#define ShowPlayer_m_HeroName (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "ShowPlayer", "m_HeroName")

//Class LogicFighter
#define LogicFighter_m_SkillComp (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "m_SkillComp")

// Class LogicFighter (was LogicPlayer in older versions — m_KillWildTimes now
// lives on Battle.LogicFighter and its subclasses)
#define LogicPlayer_m_KillWildTimes (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "m_KillWildTimes")

//Class LogicSkillComp
#define LogicSkillComp_m_CoolDownComp (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "Battle", "LogicSkillComp", "m_CoolDownComp")

//Class CoolDownComp
#define CoolDownComp_m_DicCoolInfo (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "Battle", "CoolDownComp", "m_DicCoolInfo")

#define CoolDownData_GetCoolTime (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "CoolDownData", "GetCoolTime")

// Class Battle.ShowUnitAIComp — TryUseSkill hook
// v2.1.88: TryUseSkill berada di Battle.ShowUnitAIComp (BUKAN ShowPlayer).
// 16-param: (state*, skillId, dir, dirDefault, pos, bCommonAtk, bAuto, bAlong,
//   bInQueue, isInFirstDragRange, firstTarget, keyState,
//   bIgnoreQueue, dragTime, uiCastTime, uiOperflag)
// offset dari dump: 0xffffffff8c4dba64
#define ShowUnitAIComp_Update (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "ShowUnitAIComp", "Update")
#define ShowUnitAIComp_TryUseSkill (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "ShowUnitAIComp", "TryUseSkill", 16)

//Class Bullet
#define Bullet_m_Id (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "Battle", "Bullet", "m_Id")
#define Bullet_m_Target (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "Battle", "Bullet", "m_Target")
#define Bullet_transform (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "Battle", "Bullet", "transform")

//Unlock Skin -> Mencoba
//Class UIRankHero.ChangeShow
#define UIRankHero_ChangeShow_iSelfHero (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "UIRankHero/ChangeShow", "iSelfHero")

//Class BattlePlayerInfo
#define BattlePlayerInfo_lUid (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "MTTDProto", "BattlePlayerInfo", "lUid")
#define BattlePlayerInfo_uiSelHero (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "MTTDProto", "BattlePlayerInfo", "uiSelHero")
#define BattlePlayerInfo_uiSkinId (uintptr_t) Il2CppGetFieldOffset("Assembly-CSharp.dll", "MTTDProto", "BattlePlayerInfo", "uiSkinId")

//Class CmdHeroSkin
#define CmdHeroSkin__ctor (uintptr_t) (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "MTTDProto", "CmdHeroSkin", ".ctor")

//Class CmdHeroStatue
#define CmdHeroStatue__ctor (uintptr_t) (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "MTTDProto", "CmdHeroStatue", ".ctor")

//Class SystemData
#define SystemData_GetHeroSkin (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "SystemData", "GetHeroSkin", 2)
#define SystemData_GetMCLimitSkin (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "SystemData", "GetMCLimitSkin", 1)
#define SystemData_GetHeroHolyStatue (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "SystemData", "GetHeroHolyStatue", 2)
#define SystemData_IsCanUseSkin (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "SystemData", "IsCanUseSkin", 1)
#define SystemData_IsHaveSkin (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "SystemData", "IsHaveSkin", 1)
#define SystemData_IsHaveSkinForever (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "SystemData", "IsHaveSkinForever", 1)
#define SystemData_IsHaveStatue (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "SystemData", "IsHaveStatue", 1)
#define SystemData_IsHaveStatueForever (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "SystemData", "IsHaveStatueForever", 1)
#define SystemData_IsForbidStatue (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "SystemData", "IsForbidStatue", 1)
#define SystemData_IsForbidSkin (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "SystemData", "IsForbidSkin", 1)
#define SystemData_IsForbidARSkin (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "SystemData", "IsForbidARSkin", 1)

//Class UIChooseHero
#define UIChooseHero_SendSelectSkin (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "UIChooseHero", "SendSelectSkin", 2)

//Class UIRankHero
#define UIRankHero_BatttleSelectSkin (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "UIRankHero", "BatttleSelectSkin", 2)

//Class UIRankHero.ChangeShow
#define UIRankHero_ChangeShow_SendUseSkin (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "UIRankHero/ChangeShow", "SendUseSkin", 2)

//Class BattleReceiveMessage
#define BattleReceiveMessage_SetPlayerData (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "BattleReceiveMessage", "SetPlayerData", 2)
#define BattleReceiveMessage_SetPlayerData_ (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "BattleReceiveMessage", "SetPlayerData", 1)
#define BattleReceiveMessage_AddPlayerInfo (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "BattleReceiveMessage", "AddPlayerInfo", 4)
#define BattleReceiveMessage_AddPlayerInfo_ (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "BattleReceiveMessage", "AddPlayerInfo", 2)

//Class GameServerConfig
#define GameServerConfig_SendRawData (uintptr_t) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "GameServerConfig", "SendRawData", 7)
