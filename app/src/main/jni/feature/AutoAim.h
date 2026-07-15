#pragma once

struct sAim {
    struct sHelper {
        struct sSkills {
            bool Basic;
            bool Spell;
            bool Skill1;
            bool Skill2;
            bool Skill3;
            bool Skill4;
        };
        sSkills Skills{0};

        struct sPriority {
            int Target = 0;
            bool AutoTakeSword;
        };
        sPriority Priority{0};
		
		struct sAutoRetribution {
			bool Buff;
            bool Turtle;
			bool Lord;
        };
        sAutoRetribution AutoRetribution{0};
		
    };
    sHelper Helper{0};
};
sAim Aim{0};

static float RangeFOV = 10.0f;

// UPDATED v2.1.88: TryUseSkill moved from ShowUnitAIComp to ShowPlayer.
// New 9-param overload: (skillId, dir, dirDefault, pos, bCommonAttack,
//                        bAlong, isInFirstDragRange, bIgnoreQueue, dragTime)
// Return type is SPELL_CAST_RESULT (int), bAuto removed, 3 new params added.
int (*orig_TryUseSkill)(...);
int TryUseSkill(void *instance, int skillId, Vector3 dir, bool dirDefault,
                Vector3 pos, bool bCommonAtk, bool bAlong,
                bool isInFirstDragRange, bool bIgnoreQueue, unsigned int dragTime) {
	bool isDoneAim = false;
    if (instance != NULL) {
        float MaxDist = std::numeric_limits<float>::infinity();
        float MaxEnemyHP = std::numeric_limits<float>::infinity();
        float MaxPercentHP = std::numeric_limits<float>::infinity();
        float MaxSwordDist = std::numeric_limits<float>::infinity();
        Vector3 EntityPos = Vector3::zero();
        Vector3 SwordPos = Vector3::zero();

        void *BattleManager_Instance;
        Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "BattleManager", "Instance", &BattleManager_Instance);
        if (BattleManager_Instance) {
            auto m_LocalPlayerShow = *(uintptr_t *) ((uintptr_t)BattleManager_Instance + BattleManager_m_LocalPlayerShow);
            if (m_LocalPlayerShow) {
                auto selfPos = *(Vector3 *) ((uintptr_t)m_LocalPlayerShow + ShowEntity__Position);
                auto HeroID = *(int *) ((uintptr_t)m_LocalPlayerShow + EntityBase_m_ID);
                if (selfPos != Vector3::zero()) {
                    monoList<void **> *m_ShowPlayers = *(monoList<void **> **) ((uintptr_t)BattleManager_Instance + BattleManager_m_ShowPlayers);
                    if (m_ShowPlayers) {
                        for (int i = 0; i < m_ShowPlayers->getSize(); i++) {
                            auto Pawn = m_ShowPlayers->getItems()[i];
                            if (!Pawn) continue;
                            auto m_bSameCampType = *(bool *) ((uintptr_t)Pawn + EntityBase_m_bSameCampType);
                            if (m_bSameCampType) continue;
                            auto m_bDeath = *(bool *) ((uintptr_t)Pawn + EntityBase_m_bDeath);
                            if (m_bDeath) continue;
                            auto _Position = *(Vector3 *) ((uintptr_t)Pawn + ShowEntity__Position);
                            float Distance = (int) Vector3::Distance(selfPos, _Position);
                            float CurHP = *(int *) ((uintptr_t)Pawn + EntityBase_m_Hp);
                            float MaxHP = *(int *) ((uintptr_t)Pawn + EntityBase_m_HpMax);
                            float PercentHP = CurHP * 100 / MaxHP;
                            if (Distance < RangeFOV) {
                                if (Aim.Helper.Priority.Target == 0 && Distance < MaxDist) {
                                    EntityPos = _Position;
                                    MaxDist = Distance;
                                }
                                if (Aim.Helper.Priority.Target == 1 && CurHP < MaxEnemyHP) {
                                    EntityPos = _Position;
                                    MaxEnemyHP = CurHP;
                                }
                                if (Aim.Helper.Priority.Target == 2 && PercentHP < MaxPercentHP) {
                                    EntityPos = _Position;
                                    MaxPercentHP = PercentHP;
                                }
                            }
                        }
                    }

                    monoList<void **> *_RunBullets = *(monoList<void **> **) ((uintptr_t)BattleManager__RunBullets);
                    if (_RunBullets) {
                        for (int i = 0; i < _RunBullets->getSize(); i++) {
                            auto Pawn = _RunBullets->getItems()[i];
                            if (!Pawn) continue;
                            auto m_ID = *(int *) ((uintptr_t)Pawn + Bullet_m_Id);
                            if (m_ID != 8452) continue;
                            auto transform = *(Transform **) ((uintptr_t)Pawn + Bullet_transform);
                            float Distance = (int) Vector3::Distance(selfPos, transform->get_position());
                            if (Distance < 5) {
                                if (Aim.Helper.Priority.AutoTakeSword && Distance < MaxSwordDist) {
                                    SwordPos = transform->get_position();
                                    MaxSwordDist = Distance;
                                }
                            }
                        }
                    }

                    if (SwordPos != Vector3::zero() && HeroID == 84) {
                        auto targetLockPos = Vector3::Normalized(SwordPos - selfPos);
                        if (skillId == 100 * HeroID + 20) {
                            isDoneAim = true;
							orig_TryUseSkill(instance, skillId, targetLockPos, dirDefault, pos, bCommonAtk, bAlong, isInFirstDragRange, bIgnoreQueue, dragTime);
                        }
                    } else if (EntityPos != Vector3::zero()) {
                        auto targetLockPos = Vector3::Normalized(EntityPos - selfPos);
                        //Basic
                        if (Aim.Helper.Skills.Basic) {
                            if (skillId == 100 * HeroID + 00) {
                                isDoneAim = true;
                                orig_TryUseSkill(instance, skillId, targetLockPos, dirDefault, pos, bCommonAtk, bAlong, isInFirstDragRange, bIgnoreQueue, dragTime);
                            }
                        }
                        //Spell
                        if (Aim.Helper.Skills.Spell) {
                            if (skillId == 20100 || skillId == 20140) {
                                isDoneAim = true;
                                orig_TryUseSkill(instance, skillId, targetLockPos, dirDefault, pos, bCommonAtk, bAlong, isInFirstDragRange, bIgnoreQueue, dragTime);
                            }
                        }
                        //Skill 1
                        if (Aim.Helper.Skills.Skill1) {
                            if (skillId == 100 * HeroID + 10) {
                                isDoneAim = true;
                                orig_TryUseSkill(instance, skillId, targetLockPos, dirDefault, pos, bCommonAtk, bAlong, isInFirstDragRange, bIgnoreQueue, dragTime);
                            }
                        }
                        //Skill 2
                        if (Aim.Helper.Skills.Skill2) {
                            if (skillId == 100 * HeroID + 20 || skillId == 2010520 /*Beatrix Skill2*/) {
                                isDoneAim = true;
                                orig_TryUseSkill(instance, skillId, targetLockPos, dirDefault, pos, bCommonAtk, bAlong, isInFirstDragRange, bIgnoreQueue, dragTime);
                            }
                        }
                        //Skill 3
                        if (Aim.Helper.Skills.Skill3) {
                            if (skillId == 100 * HeroID + 30 || skillId == 2010530 /*Beatrix Ulti*/) {
                                isDoneAim = true;
                                orig_TryUseSkill(instance, skillId, targetLockPos, dirDefault, pos, bCommonAtk, bAlong, isInFirstDragRange, bIgnoreQueue, dragTime);
                            }
                        }
                        //Skill 4
                        if (Aim.Helper.Skills.Skill4) {
                            if (skillId == 100 * HeroID + 40) {
                                isDoneAim = true;
                                orig_TryUseSkill(instance, skillId, targetLockPos, dirDefault, pos, bCommonAtk, bAlong, isInFirstDragRange, bIgnoreQueue, dragTime);
                            }
                        }
                    }

                }
            }
        }
    }
    if (!isDoneAim) {
        return orig_TryUseSkill(instance, skillId, dir, dirDefault, pos, bCommonAtk, bAlong, isInFirstDragRange, bIgnoreQueue, dragTime);
    }
    return 0;
}

int CalculateRetriDamage(int m_Level, int m_KillWildTimes) {
    if (m_KillWildTimes < 5) {
        return 520 + (80 * m_Level);
    } else {
        return 1.5 * (520 + (80 * m_Level));
    }
}

void (*orig_UpdateRetribution)(void *instance);
void UpdateRetribution(void *instance) {
    if (Aim.Helper.AutoRetribution.Buff || Aim.Helper.AutoRetribution.Lord || Aim.Helper.AutoRetribution.Turtle) {
        if (instance != NULL) {
            void *BattleManager_Instance;
            Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "BattleManager", "Instance", &BattleManager_Instance);
            if (BattleManager_Instance) {
                auto m_LocalPlayerShow = *(uintptr_t *) ((uintptr_t)BattleManager_Instance + BattleManager_m_LocalPlayerShow);
                if (m_LocalPlayerShow) {
                    auto selfPos = *(Vector3 *) ((uintptr_t)m_LocalPlayerShow + ShowEntity__Position);
                    if (selfPos != Vector3::zero()) {
                        auto m_Level = *(int *) ((uintptr_t)m_LocalPlayerShow + EntityBase_m_Level);
                        auto _logicFighter = *(uintptr_t *) ((uintptr_t)m_LocalPlayerShow + ShowEntity__logicFighter);
                        int iCalculateDamage = 520 + (80 * m_Level); // safe default
                        if (_logicFighter) {
                            auto m_KillWildTimes = *(int *) ((uintptr_t)_logicFighter + LogicPlayer_m_KillWildTimes);
                            iCalculateDamage = CalculateRetriDamage(m_Level, m_KillWildTimes);
                        }
                        monoList<void **> *m_ShowMonsters = *(monoList<void **> **) ((uintptr_t)BattleManager_Instance + BattleManager_m_ShowMonsters);
                        if (m_ShowMonsters) {
                            for (int i = 0; i < m_ShowMonsters->getSize(); i++) {
                                auto Pawn = m_ShowMonsters->getItems()[i];
                                if (!Pawn) continue;
                                auto m_ID = *(int *) ((uintptr_t)Pawn + EntityBase_m_ID);
                                if (MonsterToString(m_ID) == "") continue;
                                auto m_bSameCampType = *(bool *) ((uintptr_t)Pawn + EntityBase_m_bSameCampType);
                                if (m_bSameCampType) continue;
                                auto m_bDeath = *(bool *) ((uintptr_t)Pawn + EntityBase_m_bDeath);
                                if (m_bDeath) continue;
                                auto _Position = *(Vector3 *) ((uintptr_t)Pawn + ShowEntity__Position);
                                float Distance = (int) Vector3::Distance(selfPos, _Position);
                                float CurHP = *(int *) ((uintptr_t)Pawn + EntityBase_m_Hp);
                                float MaxHP = *(int *) ((uintptr_t)Pawn + EntityBase_m_HpMax);
                                if (Distance < 2.5f && CurHP < MaxHP) {
                                    if (CurHP <= iCalculateDamage) {
                                        if (m_ID == 2004 && Aim.Helper.AutoRetribution.Buff /*Red Buff*/
                                         || m_ID == 2005 && Aim.Helper.AutoRetribution.Buff /*Blue Buff*/
                                         || m_ID == 2002 && Aim.Helper.AutoRetribution.Lord  /*Lord*/
                                         || m_ID == 2003 && Aim.Helper.AutoRetribution.Turtle /*Turtle*/) {
                                            if (Vector3::Normalized(_Position - selfPos) != Vector3::zero()) {
                                                // Retribution skill: use spell slot 20020 with new signature
                                                orig_TryUseSkill(instance, 20020,
                                                    Vector3::Normalized(_Position - selfPos),
                                                    true, Vector3::zero(),
                                                    false, false, false, false, 0u);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    orig_UpdateRetribution(instance);
}
