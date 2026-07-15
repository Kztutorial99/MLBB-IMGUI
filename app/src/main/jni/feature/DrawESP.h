#pragma once

struct sConfig {
    struct sESP {
        struct sPlayer {
            bool Line;
            bool Box;
            bool Health;
            bool Name;
            bool Distance;
            bool Hero;
            bool CoolDown;
            bool Alert;
            bool Visible;
        };
        sPlayer Player{0};

        struct sMonster {
            bool Box;
            bool Health;
            bool Name;
            bool Alert;
        };
        sMonster Monster{0};
    };
    sESP ESP{0};

    struct sVisual {
        bool UnlockSkin;
        bool MaphackIcon;
		bool HealthBar;
    };
    sVisual Visual{0};
};
sConfig Config{0};

bool isOutsideScreen(ImVec2 pos, ImVec2 screen) {
    if (pos.y < 0) {
        return true;
    }
    if (pos.x > screen.x) {
        return true;
    }
    if (pos.y > screen.y) {
        return true;
    }
    return pos.x < 0;
}

ImVec2 pushToScreenBorder(ImVec2 Pos, ImVec2 screen, int offset) {
    int x = (int) Pos.x;
    int y = (int) Pos.y;
    if (Pos.y < 0) {
        // top
        y = -offset;
    }
    if (Pos.x > screen.x) {
        // right
        x = (int) screen.x + offset;
    }
    if (Pos.y > screen.y) {
        // bottom
        y = (int) screen.y + offset;
    }
    if (Pos.x < 0) {
        // left
        x = -offset;
    }
    return ImVec2(x, y);
}

void DrawCircleHealth(ImVec2 position, int health, int max_health, float radius) {
    float a_max = ((3.14159265359f * 2.0f));
    ImU32 healthColor = IM_COL32(0, 160, 0, 230); // GREEN
    if (health <= (max_health * 0.6)) {
        healthColor = IM_COL32(255, 255, 0, 110); // YELLOW
    }
    if (health < (max_health * 0.3)) {
        healthColor = IM_COL32(255, 0, 0, 255); // RED
    }
    ImGui::GetForegroundDrawList()->PathArcTo(position, radius, (-(a_max / 4.0f)) + (a_max / max_health) * (max_health - health), a_max - (a_max / 4.0f));
    ImGui::GetForegroundDrawList()->PathStroke(healthColor, ImDrawFlags_None, 4);
}

static float SetFieldOfView = 0, GetFieldOfView = 0;

void DroneView() {
    if (GetFieldOfView == 0) {
        GetFieldOfView = get_fieldOfView();
    }
    if (SetFieldOfView > 0 && GetFieldOfView != 0) {
        set_fieldOfView((float)GetFieldOfView + SetFieldOfView);
    }
    if (SetFieldOfView == 0 && GetFieldOfView != 0) {
        set_fieldOfView((float)GetFieldOfView);
    }
}

//MapHack With Hook Update (No FC/Lag) -> Blink
void (*orig_UpdateMapHack)(void *instance);
void UpdateMapHack(void *instance) {
    if (instance != NULL && Config.Visual.MaphackIcon || instance != NULL && Config.Visual.HealthBar) {
        void *BattleBridge_Instance, *BattleManager_Instance;
        Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "BattleData", "m_BattleBridge", &BattleBridge_Instance);
        if (BattleBridge_Instance) {
            Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "BattleManager", "Instance", &BattleManager_Instance);
            if (BattleManager_Instance) {
                monoList<void **> *m_ShowPlayers = *(monoList<void **> **) ((uintptr_t)BattleManager_Instance + BattleManager_m_ShowPlayers);
                if (m_ShowPlayers) {
                    for (int i = 0; i < m_ShowPlayers->getSize(); i++) {
                        auto Pawn = m_ShowPlayers->getItems()[i];
                        if (!Pawn) continue;
                        auto m_bSameCampType = *(bool *) ((uintptr_t)Pawn + EntityBase_m_bSameCampType);
                        if (m_bSameCampType) continue;
                        auto m_bDeath = *(bool *) ((uintptr_t)Pawn + EntityBase_m_bDeath);
                        auto m_IsDeathProgress = *(bool *) ((uintptr_t)Pawn + EntityBase_m_IsDeathProgress);
                        if (m_bDeath || m_IsDeathProgress) continue;
                        auto canSight = *(bool *) ((uintptr_t)Pawn + EntityBase_canSight);
                        if (canSight) continue;
                        auto m_uGuid = *(int *) ((uintptr_t)Pawn + EntityBase_m_uGuid);
                        auto _Position = *(Vector3 *) ((uintptr_t)Pawn + ShowEntity__Position);
						if (Config.Visual.MaphackIcon) {
                            auto SetMapEntityIconPos = (void (*)(void *, Vector3, int, bool)) (BattleBridge_SetMapEntityIconPos);
                            auto SetMapInvisibility = (void (*)(void *, int, bool, bool)) (BattleBridge_SetMapInvisibility);
                            SetMapInvisibility(BattleBridge_Instance, m_uGuid, true, m_bDeath);
                            SetMapEntityIconPos(BattleBridge_Instance, _Position, m_uGuid, true);
						}
						if (Config.Visual.HealthBar) {
							auto SetBloodInvisibility = (void (*)(void *, int, bool, bool, bool)) (BattleBridge_SetBloodInvisibility);
                            auto SynBloodAttr = (void (*)(void *, void *, bool)) (BattleBridge_SynBloodAttr);
                            auto UpdateBloodPos = (void (*)(void *, void *, bool)) (BattleBridge_UpdateBloodPos);
							SetBloodInvisibility(BattleBridge_Instance, m_uGuid, true, m_bDeath, true);
                            SynBloodAttr(BattleBridge_Instance, Pawn, true);
                            UpdateBloodPos(BattleBridge_Instance, Pawn, true);
						}
                    }
                }
            }
        }
    }
    orig_UpdateMapHack(instance);
}

//Draw ESP
void DrawESP(ImDrawList *draw, int screenWidth, int screenHeight, float screenDPi) {
    void *BattleBridge_Instance, *BattleManager_Instance, *LogicBattleManager_Instance;
    Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "BattleData", "m_BattleBridge", &BattleBridge_Instance);
    if (BattleBridge_Instance) {
        auto _bStartBattle = *(bool *) ((uintptr_t)BattleBridge_Instance + BattleBridge_bStartBattle);
        if (_bStartBattle) {
            DroneView();
            Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "BattleManager", "Instance", &BattleManager_Instance);
            if (BattleManager_Instance) {
                auto m_LocalPlayerShow = *(uintptr_t *) ((uintptr_t)BattleManager_Instance + BattleManager_m_LocalPlayerShow);
                if (m_LocalPlayerShow) {
                    auto selfPos = *(Vector3 *) ((uintptr_t)m_LocalPlayerShow + ShowEntity__Position);
                    if (selfPos != Vector3::zero()) {
                        auto SelfPosW2S = WorldToScreen(selfPos);
                        auto SelfPosVec2 = ImVec2(screenWidth - SelfPosW2S.x, SelfPosW2S.y);
                        if (SelfPosW2S.z > 0) {
                            SelfPosVec2 = ImVec2(SelfPosW2S.x, screenHeight - SelfPosW2S.y);
                        }
                        monoList<void **> *m_ShowPlayers = *(monoList<void **> **) ((uintptr_t)BattleManager_Instance + BattleManager_m_ShowPlayers);
                        if (m_ShowPlayers) {
                            for (int i = 0; i < m_ShowPlayers->getSize(); i++) {
                                auto Pawn = m_ShowPlayers->getItems()[i];
                                if (!Pawn) continue;
                                auto m_bSameCampType = *(bool *) ((uintptr_t)Pawn + EntityBase_m_bSameCampType);
                                if (m_bSameCampType) continue;
                                auto m_bDeath = *(bool *) ((uintptr_t)Pawn + EntityBase_m_bDeath);
                                if (m_bDeath) continue;
                                auto canSight = *(bool *) ((uintptr_t)Pawn + EntityBase_canSight);
                                auto CurHP = *(int *) ((uintptr_t)Pawn + EntityBase_m_Hp);
                                auto MaxHP = *(int *) ((uintptr_t)Pawn + EntityBase_m_HpMax);
                                auto _Position = *(Vector3 *) ((uintptr_t)Pawn + ShowEntity__Position);

                                auto *m_RoleName = *(MonoString **) ((uintptr_t)Pawn + ShowEntity_m_RoleName);
                                auto *m_HeroName = *(MonoString **) ((uintptr_t)Pawn + ShowPlayer_m_HeroName);

                                auto HeroID = *(int *) ((uintptr_t)Pawn + EntityBase_m_ID);

                                int Distance = (int) Vector3::Distance(selfPos, _Position);
                                auto RootPosW2S = WorldToScreen(_Position);
                                auto RootPosVec2 = Vector2(screenWidth - RootPosW2S.x, RootPosW2S.y);
                                if (RootPosW2S.z > 0) {
                                    RootPosVec2 = Vector2(RootPosW2S.x, screenHeight - RootPosW2S.y);
                                }
								
								
                                auto HeadPosVec2 = Vector2(RootPosVec2.x, RootPosVec2.y - (screenHeight / 6.35));

								
								//Esp Visible
                                if (Config.ESP.Player.Visible) {
                                    if (canSight && !isOutsideScreen(ImVec2(RootPosVec2.x, RootPosVec2.y), ImVec2(screenWidth, screenHeight)))
                                        continue;
                                }
								
								
								//Esp Line
                                if (Config.ESP.Player.Line) {
                                    draw->AddLine(SelfPosVec2, ImVec2(RootPosVec2.x, RootPosVec2.y), IM_COL32(255, 255, 255, 255), 1.7f);
                                }
								
								
								
								//Esp Box
                                if (Config.ESP.Player.Box) {
                                    float boxHeight = abs(HeadPosVec2.y - RootPosVec2.y);
                                    float boxWidth = boxHeight * 0.70f;
                                    ImVec2 vStart = {HeadPosVec2.x - (boxWidth / 2), HeadPosVec2.y};
                                    ImVec2 vEnd = {vStart.x + boxWidth, vStart.y + boxHeight};
                                    draw->AddRect(vStart, vEnd, IM_COL32(255, 255, 255, 255),1.7f);
                                }
								
								
								//Esp Hero Health
								if (Config.ESP.Player.Health) {
                                float boxHeight = abs(HeadPosVec2.y - RootPosVec2.y);
                                    float boxWidth = boxHeight * 0.75f;
                                    float PercentHP = ((float)CurHP * boxHeight) / (float)MaxHP;
                                    draw->AddRectFilled(ImVec2(RootPosVec2.x + (boxWidth / 2) + 5, RootPosVec2.y), ImVec2(RootPosVec2.x + (boxWidth / 2) + 15, RootPosVec2.y - PercentHP), IM_COL32(0, 160, 0, 230));
                                    draw->AddRect(ImVec2(RootPosVec2.x + (boxWidth / 2) + 5, RootPosVec2.y), ImVec2(RootPosVec2.x + (boxWidth / 2) + 15, HeadPosVec2.y), IM_COL32(255, 255, 255, 255),0.10);
                                }
								
								
								//Esp Player Name
                                if (Config.ESP.Player.Name || Config.ESP.Player.Hero) {
                                    if (Config.ESP.Player.Name && Config.ESP.Player.Hero) {
                                        if (m_RoleName && m_HeroName) {
                                            std::string strName = "(" + m_HeroName->toString() + ") " + m_RoleName->toString();
                                            auto textSize = ImGui::CalcTextSize2(strName.c_str(), 0, ((float) screenHeight / 39.0f));
                                            draw->AddText(NULL, ((float) screenHeight / 39.0f), {RootPosVec2.x - (textSize.x / 2), RootPosVec2.y + 25}, IM_COL32(255, 255, 255, 255), strName.c_str());
                                        }
										
									//Esp Player Name
                                    } else if (Config.ESP.Player.Name) {
                                        if (m_RoleName) {
                                            std::string strName = m_RoleName->toString();
                                            auto textSize = ImGui::CalcTextSize2(strName.c_str(), 0, ((float) screenHeight / 39.0f));
                                            draw->AddText(NULL, ((float) screenHeight / 39.0f), {RootPosVec2.x - (textSize.x / 2), RootPosVec2.y + 25}, IM_COL32(255, 255, 255, 255), strName.c_str());
                                        }
										
									//
                                    } else if (Config.ESP.Player.Hero) {
                                        if (m_HeroName) {
                                            std::string strName = "(" + m_HeroName->toString() + ")";
                                            auto textSize = ImGui::CalcTextSize2(strName.c_str(), 0, ((float) screenHeight / 39.0f));
                                            draw->AddText(NULL, ((float) screenHeight / 39.0f), {RootPosVec2.x - (textSize.x / 2), RootPosVec2.y + 25}, IM_COL32(255, 255, 255, 255), strName.c_str());
                                        }
                                    }
                                }
								
								
								//Esp Distance
                                if (Config.ESP.Player.Distance) {
                                    std::string strDistance = to_string(Distance) + "M";
                                    auto textSize = ImGui::CalcTextSize2(strDistance.c_str(), 0, ((float) screenHeight / 39.0f));
                                    draw->AddText(NULL, ((float) screenHeight / 39.0f), {RootPosVec2.x - (textSize.x / 2), RootPosVec2.y + 5}, IM_COL32(255, 255, 255, 255), strDistance.c_str());
                                }
                                if (Config.ESP.Player.CoolDown) {
                                    uint32_t cd[8] {0};
                                    auto _logicFighter = *(uintptr_t *) ((uintptr_t)Pawn + ShowEntity__logicFighter);
                                    if (_logicFighter) {
                                        auto m_SkillComp = *(uintptr_t *) ((uintptr_t)_logicFighter + LogicFighter_m_SkillComp);
                                        if (m_SkillComp) {
                                            auto m_CoolDownComp = *(uintptr_t *) ((uintptr_t)m_SkillComp + LogicSkillComp_m_CoolDownComp);
                                            if (m_CoolDownComp) {
                                                Dictionary<int, uintptr_t> *m_DicCoolInfo = *(Dictionary<int, uintptr_t> **) ((uintptr_t)m_CoolDownComp + CoolDownComp_m_DicCoolInfo);
                                                if (m_DicCoolInfo) {
                                                    for (int j = 0; j < m_DicCoolInfo->getNumKeys(); j++) {
                                                        auto keys = m_DicCoolInfo->getKeys()[j];
                                                        auto values = m_DicCoolInfo->getValues()[j];
                                                        if (keys && values) {
                                                            int skillIdx = -1;
                                                            //Spell
                                                            if (SpellIdExist(keys)) {
                                                                skillIdx = 4;
                                                            }
                                                            //Skill 1 different method detect
                                                            else if (keys % 100 == 10) {
                                                                skillIdx = 0;
                                                            }
                                                            //Skill 2 different method detect
                                                            else if (keys % 100 == 20) {
                                                                skillIdx = 1;
                                                            }
                                                            //Skill 3 different method detect
                                                            else if (keys % 100 == 30) {
                                                                skillIdx = 2;
                                                            }
                                                            //Skill 1
                                                            else if (keys == 100 * HeroID + 10 || keys == 2010510/*Beatrix Skill1*/) {
                                                                skillIdx = 0;
                                                            }
                                                            //Skill 2
                                                            else if (keys == 100 * HeroID + 20 || keys == 2010520/*Beatrix Skill2*/) {
                                                                skillIdx = 1;
                                                            }
                                                            //Skill 3
                                                            else if (keys == 100 * HeroID + 30 || keys == 2010530/*Beatrix Ulti*/) {
                                                                skillIdx = 2;
                                                            }
                                                            //Skill 4
                                                            else if (keys == 100 * HeroID + 40) {
                                                                skillIdx = 3;
                                                            }
                                                            if (skillIdx != -1) {
                                                                uint32_t uiCoolTime = ((uint32_t (*)(uintptr_t)) (CoolDownData_GetCoolTime))(values);
                                                                if (uiCoolTime > 0) {
                                                                    cd[skillIdx] = uiCoolTime / 1000;
                                                                } else {
                                                                    cd[skillIdx] = 0;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
									
									
									//Esp Cooldown
                                    if (cd[0] || cd[1] || cd[2] || cd[3] || cd[4]) {
                                        std::string strCooldown;
                                        if (cd[0]) {
                                            strCooldown += "Skill (1): " + to_string(cd[0]);
                                        }
                                        if (cd[1]) {
                                            if (!cd[0]) {
                                                strCooldown += "Skill (2): " + to_string(cd[1]);
                                            } else {
                                                strCooldown += " | Skill (2): " + to_string(cd[1]);
                                            }
                                        }
                                        if (cd[2]) {
                                            if (!cd[0] && !cd[1]) {
                                                strCooldown += "Skill (3): " + to_string(cd[2]);
                                            } else {
                                                strCooldown += " | Skill (3): " + to_string(cd[2]);
                                            }
                                        }
                                        if (cd[3]) {
                                            if (!cd[0] && !cd[1] && !cd[2]) {
                                                strCooldown += "Skill (4): " + to_string(cd[3]);
                                            } else {
                                                strCooldown += " | Skill (4): " + to_string(cd[3]);
                                            }
                                        }
                                        if (cd[4]) {
                                            if (!cd[0] && !cd[1] && !cd[2] && !cd[3]) {
                                                strCooldown += "Spell: " + to_string(cd[4]);
                                            } else {
                                                strCooldown += " | Spell: " + to_string(cd[4]);
                                            }
                                        }
                                        auto textSize = ImGui::CalcTextSize2(strCooldown.c_str(), 0, ((float) screenHeight / 39.0f));
                                        draw->AddText(NULL, ((float) screenHeight / 39.0f), {RootPosVec2.x - (textSize.x / 2), RootPosVec2.y + 60}, IM_COL32(255, 255, 255, 255), strCooldown.c_str());
                                    }
                                }
								
								
								//Esp Alert
                                if (Config.ESP.Player.Alert && isOutsideScreen(ImVec2(RootPosVec2.x, RootPosVec2.y), ImVec2(screenWidth, screenHeight))) {
                                    ImVec2 hintDotRenderPos = pushToScreenBorder(ImVec2(RootPosVec2.x, RootPosVec2.y), ImVec2(screenWidth, screenHeight), - 50);
                                    ImVec2 hintTextRenderPos = pushToScreenBorder(ImVec2(RootPosVec2.x, RootPosVec2.y), ImVec2(screenWidth, screenHeight), - 50);
                                    draw->AddCircleFilled(hintDotRenderPos, 25, IM_COL32(255, 0, 0, 110));
                                    DrawCircleHealth(hintDotRenderPos, CurHP, MaxHP, 25);
                                    std::string strDistance = to_string(Distance) + " M";
                                    auto textSize = ImGui::CalcTextSize2(strDistance.c_str(), 0, ((float) screenHeight / 45.0f));
                                    draw->AddText(NULL, ((float) screenHeight / 45.0f), {hintTextRenderPos.x - (textSize.x / 2), hintTextRenderPos.y - 7}, IM_COL32(255, 255, 255, 255), strDistance.c_str());
                                    if (m_HeroName) {
                                        std::string strName = m_HeroName->toString();
                                        auto textSize = ImGui::CalcTextSize2(strName.c_str(), 0, ((float) screenHeight / 39.0f));
                                        draw->AddText(NULL, ((float) screenHeight / 39.0f), {hintTextRenderPos.x - (textSize.x / 2), hintTextRenderPos.y + 30}, IM_COL32(255, 255, 255, 255), strName.c_str());
                                    }
                                }
                            }
                        }
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
                            auto CurHP = *(int *) ((uintptr_t)Pawn + EntityBase_m_Hp);
                            auto MaxHP = *(int *) ((uintptr_t)Pawn + EntityBase_m_HpMax);
                            auto _Position = *(Vector3 *) ((uintptr_t)Pawn + ShowEntity__Position);
                            auto RootPosW2S = WorldToScreen(_Position);
                            auto RootPosVec2 = Vector2(screenWidth - RootPosW2S.x, RootPosW2S.y);
                            if (RootPosW2S.z > 0) {
                                RootPosVec2 = Vector2(RootPosW2S.x, screenHeight - RootPosW2S.y);
                            }
                            auto HeadPosVec2 = Vector2(RootPosVec2.x, RootPosVec2.y - (screenHeight / 6.35));
                            if (Config.ESP.Monster.Box) {
                                float boxHeight = abs(HeadPosVec2.y - RootPosVec2.y);
                                float boxWidth = boxHeight * 0.75f;
                                ImVec2 vStart = {HeadPosVec2.x - (boxWidth / 2), HeadPosVec2.y};
                                ImVec2 vEnd = {vStart.x + boxWidth, vStart.y + boxHeight};
                                draw->AddRect(vStart, vEnd, IM_COL32(255, 255, 255, 255),1.7f);
                            }
                            if (Config.ESP.Monster.Health) {
                                float boxHeight = abs(HeadPosVec2.y - RootPosVec2.y);
                                float boxWidth = boxHeight * 0.75f;
                                float PercentHP = ((float)CurHP * boxHeight) / (float)MaxHP;
                                draw->AddRectFilled(ImVec2(RootPosVec2.x + (boxWidth / 2) + 5, RootPosVec2.y), ImVec2(RootPosVec2.x + (boxWidth / 2) + 15, RootPosVec2.y - PercentHP), IM_COL32(255, 100, 0, 255));
                                draw->AddRect(ImVec2(RootPosVec2.x + (boxWidth / 2) + 5, RootPosVec2.y), ImVec2(RootPosVec2.x + (boxWidth / 2) + 15, HeadPosVec2.y), IM_COL32(0, 0, 0, 255), 0.5);
                            }
                            if (Config.ESP.Monster.Name) {
                                std::string strName = MonsterToString(m_ID);
                                auto textSize = ImGui::CalcTextSize2(strName.c_str(), 0, ((float) screenHeight / 39.0f));
                                draw->AddText(NULL, ((float) screenHeight / 39.0f), {RootPosVec2.x - (textSize.x / 2), RootPosVec2.y + 25}, IM_COL32(255, 255, 255, 255), strName.c_str());
                            }
                            if (Config.ESP.Monster.Alert) {
                                if (m_ID == 2002 && CurHP < MaxHP) {
                                    std::string strAlert = "[ALERT] Lord is under attack!";
                                    auto textSize = ImGui::CalcTextSize2(strAlert.c_str(), 0, ((float) screenHeight / 31.0f));
                                    draw->AddText(NULL, ((float) screenHeight / 31.0f), {(float)(screenWidth / 5) - (textSize.x / 2), (float)(screenHeight - screenHeight) + (float)(screenHeight / 8.7f)}, IM_COL32(255, 255, 255, 255), strAlert.c_str());
                                }
                                if (m_ID == 2003 && CurHP < MaxHP) {
                                    std::string strAlert = "[ALERT] Turtle is under attack!";
                                    auto textSize = ImGui::CalcTextSize2(strAlert.c_str(), 0, ((float) screenHeight / 31.0f));
                                    draw->AddText(NULL, ((float) screenHeight / 31.0f), {(float)(screenWidth / 2) - (textSize.x / 2), (float)(screenHeight - screenHeight) + (float)(screenHeight / 8.7f)}, IM_COL32(255, 255, 255, 255), strAlert.c_str());
                                }
                            }
                        }
                    }

                }
            }
        }
    }
}
