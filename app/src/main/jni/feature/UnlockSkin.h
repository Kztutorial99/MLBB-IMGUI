#pragma once

#define DefineHook(RET, NAME, ARGS) \
    RET (*o ## NAME) ARGS; \
    RET NAME ARGS

struct CmdHeroSkin : Il2CppObject {
    uint32_t iId;
    uint32_t iLimitTime;
    uint32_t iSource;
};

struct CmdHeroStatue : Il2CppObject {
    uint32_t iId;
    uint32_t iLimitTime;
    uint32_t iSource;
};

bool ndUnlockSkin = false, ndUnlockSkin2 = false;

void UnlockSkin() {
	if (Config.Visual.UnlockSkin) {
		ndUnlockSkin = true;
		ndUnlockSkin2 = true;
	} else {
		ndUnlockSkin = false;
		ndUnlockSkin2 = false;
	}
}

DefineHook(CmdHeroSkin *, GetHeroSkin, (void * unk, uintptr_t m_heroskins, uint skinid)) {
    auto ret = oGetHeroSkin(unk, m_heroskins, skinid);

    if (ret||!ndUnlockSkin)
        return ret;

    auto MTTDProto_CmdHeroSkin_ctor = (void *(*)(CmdHeroSkin *)) (CmdHeroSkin__ctor);
    auto instance = (CmdHeroSkin *) Il2CppCreateClassInstance("Assembly-CSharp.dll", "MTTDProto", "CmdHeroSkin");
    if (MTTDProto_CmdHeroSkin_ctor(instance)) {
        instance->iId = skinid;
        instance->iLimitTime = 0;
        instance->iSource = 0;
    }
    return instance;
}

DefineHook(CmdHeroSkin *, GetMCLimitSkin, (void * unk, uint skinId)) {
    auto ret = oGetMCLimitSkin(unk, skinId);

    if (ret||!ndUnlockSkin)
        return ret;

    auto  MTTDProto_CmdHeroSkin_ctor = (void *(*)(CmdHeroSkin *)) (CmdHeroSkin__ctor);
    auto instance = (CmdHeroSkin *) Il2CppCreateClassInstance("Assembly-CSharp.dll", "MTTDProto", "CmdHeroSkin");
    if (MTTDProto_CmdHeroSkin_ctor(instance)) {
        instance->iId = skinId;
        instance->iLimitTime = 0;
        instance->iSource = 0;
    }
    return instance;
}

DefineHook(CmdHeroStatue *, GetHeroHolyStatue, (void * unk, uintptr_t m_heroStatues, uint statueid)) {
    auto ret = oGetHeroHolyStatue(unk, m_heroStatues, statueid);

    if (ret||!ndUnlockSkin)
        return ret;

    auto MTTDProto_CmdHeroStatue_ctor = (void *(*)(CmdHeroStatue *)) (CmdHeroStatue__ctor);
    auto instance = (CmdHeroStatue *) Il2CppCreateClassInstance("Assembly-CSharp.dll", "MTTDProto", "CmdHeroStatue");
    if (MTTDProto_CmdHeroStatue_ctor(instance)) {
        instance->iId = statueid;
        instance->iLimitTime = 0;
        instance->iSource = 0;
    }
    return instance;
}

DefineHook(bool, IsCanUseSkin, (void * unk, int heroid)) {
    return true;
}

DefineHook(CmdHeroSkin *, IsHaveSkin, (void * unk, int skinid)) {
    auto ret = oIsHaveSkin(unk, skinid);

    if (ret||!ndUnlockSkin)
        return ret;

    auto MTTDProto_CmdHeroSkin_ctor = (void *(*)(CmdHeroSkin *)) (CmdHeroSkin__ctor);
    auto instance = (CmdHeroSkin *) Il2CppCreateClassInstance("Assembly-CSharp.dll", "MTTDProto", "CmdHeroSkin");
    if (MTTDProto_CmdHeroSkin_ctor(instance)) {
        instance->iId = skinid;
        instance->iLimitTime = 0;
        instance->iSource = 0;
    }
    return instance;
}

DefineHook(CmdHeroSkin *, IsHaveSkinForever, (void * unk, int skinid)) {
    auto ret = oIsHaveSkinForever(unk, skinid);

    if (ret||!ndUnlockSkin)
        return ret;

    auto MTTDProto_CmdHeroSkin_ctor = (void *(*)(CmdHeroSkin *)) (CmdHeroSkin__ctor);
    auto instance = (CmdHeroSkin *) Il2CppCreateClassInstance("Assembly-CSharp.dll", "MTTDProto", "CmdHeroSkin");
    if (MTTDProto_CmdHeroSkin_ctor(instance)) {
        instance->iId = skinid;
        instance->iLimitTime = 0;
        instance->iSource = 0;
    }
    return instance;
}

DefineHook(CmdHeroStatue *, IsHaveStatue, (void * unk, uint32_t statueid)) {
    auto ret = oIsHaveStatue(unk, statueid);

    if (ret||!ndUnlockSkin)
        return ret;

    auto MTTDProto_CmdHeroStatue_ctor = (void *(*)(CmdHeroStatue *)) (CmdHeroStatue__ctor);
    auto instance = (CmdHeroStatue *) Il2CppCreateClassInstance("Assembly-CSharp.dll", "MTTDProto", "CmdHeroStatue");
    if (MTTDProto_CmdHeroStatue_ctor(instance)) {
        instance->iId = statueid;
        instance->iLimitTime = 0;
        instance->iSource = 0;
    }
    return instance;
}

DefineHook(CmdHeroStatue *, IsHaveStatueForever, (void * unk, uint32_t statueid)) {
    auto ret = oIsHaveStatueForever(unk, statueid);

    if (ret||!ndUnlockSkin)
        return ret;

    auto MTTDProto_CmdHeroStatue_ctor = (void *(*)(CmdHeroStatue *)) (CmdHeroStatue__ctor);
    auto instance = (CmdHeroStatue *) Il2CppCreateClassInstance("Assembly-CSharp.dll", "MTTDProto", "CmdHeroStatue");
    if (MTTDProto_CmdHeroStatue_ctor(instance)) {
        instance->iId = statueid;
        instance->iLimitTime = 0;
        instance->iSource = 0;
    }
    return instance;
}

DefineHook(bool, IsForbidStatue, (void * unk, uint32_t statueid)) {
    return false;
}

DefineHook(bool, IsForbidSkin, (void * unk, uint32_t skinid)) {
    return false;
}

DefineHook(bool, IsForbidARSkin, (void * unk, uint32_t id)) {
    return false;
}

uint32_t m_HeroID = 0, m_SkinID = 0;
DefineHook(void, SendSelectSkin, (uintptr_t thiz, uint32_t skinid, uint32_t heroid)) {
    if (oIsHaveSkin(0, skinid) || oIsHaveSkinForever(0, skinid) || !ndUnlockSkin) {
        m_SkinID = 0;
        return oSendSelectSkin(thiz, skinid, heroid);
    }

    auto UIChooseHero_BatttleSelectSkin = (void (*)(uintptr_t, uint64_t, uint32_t)) (Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "UIChooseHero", "BatttleSelectSkin", 2));

    uint64_t m_uiID;
    Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "SystemData", "m_uiID", &m_uiID);
    UIChooseHero_BatttleSelectSkin(thiz, m_uiID, skinid);

    m_HeroID = heroid;
    m_SkinID = skinid;
}

DefineHook(void, BatttleSelectSkin, (uintptr_t thiz, uint64_t uid, uint skinid)) {
    if (ndUnlockSkin2) {
        uint64_t m_uiID;
        Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "SystemData", "m_uiID", &m_uiID);
        return oBatttleSelectSkin(thiz, m_uiID, m_SkinID);
    }
}

DefineHook(void, SendUseSkin, (void * thiz, uint32_t skinid, bool)) {
    if (oIsHaveSkin(0, skinid) || oIsHaveSkinForever(0, skinid) || !ndUnlockSkin) {
        m_SkinID = 0;
        return oSendUseSkin(thiz, skinid, true);
    }
    m_HeroID = *(uint32_t *) ((uintptr_t) thiz + UIRankHero_ChangeShow_iSelfHero);
    m_SkinID = skinid;
}

DefineHook(void, SetPlayerData, (uintptr_t thiz, uintptr_t playerinfo, uint32_t uiSelfCamp)) {
    if (ndUnlockSkin2) {
        if (m_SkinID) {
            if (!oIsHaveSkin(0, m_SkinID) || !oIsHaveSkinForever(0, m_SkinID)) {
                uint64_t m_uiID;
                Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "SystemData", "m_uiID", &m_uiID);
                if (*(uint64_t *) (playerinfo + BattlePlayerInfo_lUid) == m_uiID) {
                    uint32_t heroid = *(uint32_t *) (playerinfo + BattlePlayerInfo_uiSelHero);
                    if (heroid == m_HeroID) {
                        *(uint32_t *) ((uintptr_t) playerinfo + BattlePlayerInfo_uiSkinId) = m_SkinID;
                    }
                }
            }
        }
    }
    return oSetPlayerData(thiz, playerinfo, uiSelfCamp);
}

DefineHook(void, SetPlayerData_, (uintptr_t thiz, uintptr_t playerinfo)) {
    if (ndUnlockSkin2) {
        if (m_SkinID) {
            if (!oIsHaveSkin(0, m_SkinID) || !oIsHaveSkinForever(0, m_SkinID)) {
                uint64_t m_uiID;
                Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "SystemData", "m_uiID", &m_uiID);
                if (*(uint64_t *) (playerinfo + BattlePlayerInfo_lUid) == m_uiID) {
                    uint32_t heroid = *(uint32_t *) (playerinfo + BattlePlayerInfo_uiSelHero);
                    if (heroid == m_HeroID) {
                        *(uint32_t *) ((uintptr_t) playerinfo + BattlePlayerInfo_uiSkinId) = m_SkinID;
                    }
                }
            }
        }
    }
    return oSetPlayerData_(thiz, playerinfo);
}

DefineHook(void, AddPlayerInfo, (void * unk, uintptr_t playerinfo, uint selfCamp, uintptr_t roomdata, bool ignoreEmblem)) {
    if (ndUnlockSkin2) {
        if (m_SkinID) {
            if (!oIsHaveSkin(0, m_SkinID) || !oIsHaveSkinForever(0, m_SkinID)) {
                uint64_t m_uiID;
                Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "SystemData", "m_uiID", &m_uiID);
                if (*(uint64_t *) ((uintptr_t) playerinfo + BattlePlayerInfo_lUid) == m_uiID) {
                    uint32_t heroid = *(uint32_t *) ((uintptr_t) playerinfo + BattlePlayerInfo_uiSelHero);
                    if (heroid == m_HeroID) {
                        *(uint32_t *) ((uintptr_t) playerinfo + BattlePlayerInfo_uiSkinId) = m_SkinID;
                    }
                }
            }
        }
    }
    return oAddPlayerInfo(unk, playerinfo, selfCamp, roomdata, ignoreEmblem);
}

DefineHook(void, AddPlayerInfo_, (void * unk, uintptr_t playerinfo, uintptr_t roomdata)) {
    if (ndUnlockSkin2) {
        if (m_SkinID) {
            if (!oIsHaveSkin(0, m_SkinID) || !oIsHaveSkinForever(0, m_SkinID)) {
                uint64_t m_uiID;
                Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "SystemData", "m_uiID", &m_uiID);
                if (*(uint64_t *) ((uintptr_t) playerinfo + BattlePlayerInfo_lUid) == m_uiID) {
                    uint32_t heroid = *(uint32_t *) ((uintptr_t) playerinfo + BattlePlayerInfo_uiSelHero);
                    if (heroid == m_HeroID) {
                        *(uint32_t *) ((uintptr_t) playerinfo + BattlePlayerInfo_uiSkinId) = m_SkinID;
                    }
                }
            }
        }
    }
    return oAddPlayerInfo_(unk, playerinfo, roomdata);
}

// ==================================================================================================================================== //
DefineHook(void, SendRawData, (void * thiz, uint uiMsgID, Array<uint8_t> * rawData, int rawDataSize, int eSocketType, int packReliType, bool bNeedLockRequestIndex, int expectedSize)) {
    if (ndUnlockSkin2) {
        if (uiMsgID == 1015 || uiMsgID == 1016 || uiMsgID == 1019 || uiMsgID == 1020) {
            return;
        }

        if (uiMsgID == 6) {
        }
    }
    return oSendRawData(thiz, uiMsgID, rawData, rawDataSize, eSocketType, packReliType, bNeedLockRequestIndex, expectedSize);
}
// ==================================================================================================================================== //

