#include <iostream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <vector>
#include <map>
#include <chrono>
#include <fstream>
#include <thread>
#include <pthread.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <jni.h>
#include <android/log.h>
#include <elf.h>
#include <dlfcn.h>
#include <sys/system_properties.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "imgui.h"
#include "imgui_additional.h"
#include "backends/imgui_impl_android.h"
#include "backends/imgui_impl_opengl3.h"
#include "android_native_app_glue.h"
#include "fonts/GoogleSans.h"

#include "struct/Vector3.hpp"
#include "struct/Color.h"
#include "struct/Vector2.h"
#include "struct/Color.h"
#include "struct/Rect.h"
#include "struct/Quaternion.h"
#include "struct/MonoString.h"

#include "unity/fake_dlfcn.h"
#include "unity/Il2Cpp.h"
#include "unity/Tools.h"
#include "unity/Unity.h"

#include "config/Setup.h"

#include "feature/GameClass.h"
#include "feature/ToString.h"
#include "feature/RoomInfo.h"
#include "feature/DrawESP.h"
#include "feature/AutoAim.h"
#include "feature/UnlockSkin.h"

EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean _eglSwapBuffers (EGLDisplay dpy, EGLSurface surface) {
    
    eglQuerySurface(dpy, surface, EGL_WIDTH, &glWidth);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &glHeight);
if (!g_Initialized) {
          ImGui::CreateContext();
        ImGuiStyle *style = &ImGui::GetStyle();
                   
        style->WindowRounding = 4.0f;
        style->FrameRounding = 3.0f;
        style->FrameBorderSize = 0.0f;
        style->WindowBorderSize = 0.0;
        style->ScrollbarSize = 4;

        style->WindowTitleAlign = ImVec2(0.5, 0.5);
        style->FramePadding = ImVec2(6, 5);

        ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text]                              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]              = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]                          = ImVec4(0.29f, 0.34f, 0.26f, 1.00f);
    colors[ImGuiCol_ChildBg]                                = ImVec4(0.29f, 0.34f, 0.26f, 1.00f);
    colors[ImGuiCol_PopupBg]                                = ImVec4(0.24f, 0.27f, 0.20f, 1.00f);
    colors[ImGuiCol_Border]                          = ImVec4(0.54f, 0.57f, 0.51f, 0.50f);
    colors[ImGuiCol_BorderShadow]              = ImVec4(0.14f, 0.16f, 0.11f, 0.52f);
    colors[ImGuiCol_FrameBg]                                = ImVec4(0.24f, 0.27f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]          = ImVec4(0.27f, 0.30f, 0.23f, 1.00f);
    colors[ImGuiCol_FrameBgActive]            = ImVec4(0.30f, 0.34f, 0.26f, 1.00f);
    colors[ImGuiCol_TitleBg]                                = ImVec4(0.24f, 0.27f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBgActive]            = ImVec4(0.29f, 0.34f, 0.26f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]          = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]                        = ImVec4(0.24f, 0.27f, 0.20f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]                    = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]            = ImVec4(0.28f, 0.32f, 0.24f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.25f, 0.30f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.23f, 0.27f, 0.21f, 1.00f);
    colors[ImGuiCol_CheckMark]                        = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
    colors[ImGuiCol_SliderGrab]                      = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]          = ImVec4(0.54f, 0.57f, 0.51f, 0.50f);
    colors[ImGuiCol_Button]                          = ImVec4(0.29f, 0.34f, 0.26f, 0.40f);
    colors[ImGuiCol_ButtonHovered]            = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
    colors[ImGuiCol_ButtonActive]              = ImVec4(0.54f, 0.57f, 0.51f, 0.50f);
    colors[ImGuiCol_Header]                          = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
    colors[ImGuiCol_HeaderHovered]            = ImVec4(0.35f, 0.42f, 0.31f, 0.6f);
    colors[ImGuiCol_HeaderActive]              = ImVec4(0.54f, 0.57f, 0.51f, 0.50f);
    colors[ImGuiCol_Separator]                        = ImVec4(0.14f, 0.16f, 0.11f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]          = ImVec4(0.54f, 0.57f, 0.51f, 1.00f);
    colors[ImGuiCol_SeparatorActive]                = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
    colors[ImGuiCol_ResizeGrip]                      = ImVec4(0.19f, 0.23f, 0.18f, 0.00f); // grip invis
    colors[ImGuiCol_ResizeGripHovered]        = ImVec4(0.54f, 0.57f, 0.51f, 1.00f);
    colors[ImGuiCol_ResizeGripActive]          = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
    colors[ImGuiCol_Tab]                                    = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
    colors[ImGuiCol_TabHovered]                      = ImVec4(0.54f, 0.57f, 0.51f, 0.78f);
    colors[ImGuiCol_TabActive]                        = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocused]              = ImVec4(0.24f, 0.27f, 0.20f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]      = ImVec4(0.35f, 0.42f, 0.31f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]          = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
    colors[ImGuiCol_PlotHistogram]            = ImVec4(1.00f, 0.78f, 0.28f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]          = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
    colors[ImGuiCol_DragDropTarget]          = ImVec4(0.73f, 0.67f, 0.24f, 1.00f);
    colors[ImGuiCol_NavHighlight]              = ImVec4(0.59f, 0.54f, 0.18f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]        = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]          = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
        
        
        ImGuiIO* io = &ImGui::GetIO();
        io->IniFilename = nullptr;

        ImGui_ImplAndroid_Init();
        ImGui_ImplOpenGL3_Init("#version 300 es");

        ImFontConfig font_cfg;
        io->Fonts->AddFontFromMemoryCompressedTTF(GoogleSans_compressed_data, GoogleSans_compressed_size, 39.0f/*(float) get_dpi() / 14.0f*/, &font_cfg);

        g_Initialized = true;
    }

    ImGuiIO* io = &ImGui::GetIO();

    /*get touch from unity game*/
    ImGui_GetTouch(io, get_height());

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(get_width(), get_height());
    ImGui::NewFrame();

    /*draw esp here*/
    DrawESP(ImGui::GetBackgroundDrawList(), get_width(), get_height(), get_dpi());

    /*unlock all skin here*/
    UnlockSkin();

    /*load window here*/
    ImGui::SetNextWindowSize(ImVec2((float) get_width() * 0.47f, (float) get_height() * 0.78f), ImGuiCond_Once);
    // NOTE: flags are proper function arguments — do NOT use the comma operator here.
    if (ImGui::Begin("GAMING NAJMUL MLBB MOD", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse)) {
        if(ImGui::BeginTabBar("Tab", ImGuiTabBarFlags_FittingPolicyScroll)) {
            if (ImGui::BeginTabItem("Visual Menu")) {

                ImGui::BeginGroupPanel("ESP", ImVec2(0.0f, 0.0f));

                ImGui::BeginGroupPanel("Player ESP", ImVec2(0.0f, 0.0f));
                ImGui::Checkbox("Line", &Config.ESP.Player.Line);
                ImGui::Checkbox("Box", &Config.ESP.Player.Box);
                ImGui::Checkbox("Health", &Config.ESP.Player.Health);
                ImGui::Checkbox("Name", &Config.ESP.Player.Name);
                ImGui::Checkbox("Distance", &Config.ESP.Player.Distance);
                ImGui::Checkbox("Hero", &Config.ESP.Player.Hero);
                ImGui::Checkbox("CoolDown", &Config.ESP.Player.CoolDown);
                ImGui::Checkbox("Alert", &Config.ESP.Player.Alert);
                ImGui::Checkbox("Visible Check", &Config.ESP.Player.Visible);
                ImGui::Spacing();
                ImGui::EndGroupPanel();

                ImGui::SameLine();

                ImGui::BeginGroupPanel("Other", ImVec2(0.0f, 0.0f));

                ImGui::BeginGroupPanel("Monster ESP", ImVec2(0.0f, 0.0f));
                ImGui::Checkbox("Monster Box", &Config.ESP.Monster.Box);
                ImGui::Checkbox("Monster Health", &Config.ESP.Monster.Health);
                ImGui::Checkbox("Monster Name", &Config.ESP.Monster.Name);
                ImGui::Checkbox("Monster Alert", &Config.ESP.Monster.Alert);
                ImGui::Spacing();
                ImGui::EndGroupPanel();

                ImGui::BeginGroupPanel("Additional", ImVec2(0.0f, 0.0f));
                ImGui::Checkbox("Show Icon Minimap", &Config.Visual.MaphackIcon);
				ImGui::Checkbox("Show HealthBar", &Config.Visual.HealthBar);
                ImGui::Text("Drone View:");
                ImGui::SliderFloat("##DroneView", &SetFieldOfView, 0, 30, "%.1f");
                ImGui::Spacing();
                ImGui::EndGroupPanel();

                ImGui::Spacing();
                ImGui::EndGroupPanel();

                ImGui::Spacing();
                ImGui::EndGroupPanel();

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Helper Menu")) {

                ImGui::BeginGroupPanel("Auto Aim", ImVec2(0.0f, 0.0f));

                ImGui::BeginGroupPanel("Skills", ImVec2(0.0f, 0.0f));
                ImGui::Checkbox("Skill 1", &Aim.Helper.Skills.Skill1);
                ImGui::Checkbox("Skill 2", &Aim.Helper.Skills.Skill2);
                ImGui::Checkbox("Skill 3", &Aim.Helper.Skills.Skill3);
                ImGui::Checkbox("Skill 4", &Aim.Helper.Skills.Skill4);
                ImGui::Spacing();
                ImGui::EndGroupPanel();

                ImGui::SameLine();

                ImGui::BeginGroupPanel("Other", ImVec2(0.0f, 0.0f));
                ImGui::Checkbox("Basic Attack", &Aim.Helper.Skills.Basic);
                ImGui::Checkbox("Spell", &Aim.Helper.Skills.Spell);
                ImGui::Checkbox("Auto Take Sword (Ling)", &Aim.Helper.Priority.AutoTakeSword);
                ImGui::Spacing();
                ImGui::EndGroupPanel();

                ImGui::BeginGroupPanel("Priority", ImVec2(0.0f, 0.0f));
                ImGui::Text("Target Priority:");
                ImGui::RadioButton("Closest Distance", &Aim.Helper.Priority.Target, 0);
                ImGui::RadioButton("Lowest HP", &Aim.Helper.Priority.Target, 1);
                ImGui::RadioButton("Lowest HP Percentage", &Aim.Helper.Priority.Target, 2);
                ImGui::Text("Range Auto Aim:");
                ImGui::SliderFloat("##RangeFOV", &RangeFOV, 0, 150, "%.1fm");
                ImGui::Spacing();
                ImGui::EndGroupPanel();

                ImGui::SameLine();

                ImGui::BeginGroupPanel("Auto Retribution", ImVec2(0.0f, 0.0f));
                ImGui::BeginGroupPanel("Target", ImVec2(0.0f, 0.0f));
                ImGui::Checkbox("Buff", &Aim.Helper.AutoRetribution.Buff);
                ImGui::Checkbox("Turtle", &Aim.Helper.AutoRetribution.Turtle);
                ImGui::Checkbox("Lord", &Aim.Helper.AutoRetribution.Lord);
                ImGui::Spacing();
                ImGui::EndGroupPanel();
                ImGui::Spacing();
                ImGui::EndGroupPanel();
				
                ImGui::Spacing();
                ImGui::EndGroupPanel();

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Other Menu")) {
                ImGui::BeginGroupPanel("Skin", ImVec2(0.0f, 0.0f));
                ImGui::Checkbox("Unlock All Skin", &Config.Visual.UnlockSkin);
                ImGui::Spacing();
                ImGui::EndGroupPanel();

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Room Info")) {
                RoomInfoList();
                ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Team");
                if (ImGui::BeginTable("##Team", 6, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner)) {
                    ImGui::TableSetupColumn("Name", 0, 150);
                    ImGui::TableSetupColumn("User ID", 0, 160);
                    ImGui::TableSetupColumn("Squad", 0, 190);
                    ImGui::TableSetupColumn("Rank", 0, 180);
                    ImGui::TableSetupColumn("Hero", 0, 130);
                    ImGui::TableSetupColumn("Spell", 0, 120);
                    ImGui::TableHeadersRow();
                    for (int row = 0; row < 5; row++) {
                        ImGui::TableNextRow();
                        for (int column = 0; column < 6; column++) {
                            ImGui::TableSetColumnIndex(column);
                            char buf[32];
                            if (column == 0) {
                                sprintf(buf, "%s", RoomInfo.PlayerB[row].Name.c_str());
                                ImGui::TextUnformatted(buf);
                            } else if (column == 1) {
                                sprintf(buf, "%s", RoomInfo.PlayerB[row].UserID.c_str());
                                ImGui::TextUnformatted(buf);
                            } else if (column == 2) {
                                sprintf(buf, "%s", RoomInfo.PlayerB[row].Squad.c_str());
                                ImGui::TextUnformatted(buf);
                            } else if (column == 3) {
                                sprintf(buf, "%s", RoomInfo.PlayerB[row].Rank.c_str());
                                ImGui::TextUnformatted(buf);
                            } else if (column == 4) {
                                sprintf(buf, "%s", RoomInfo.PlayerB[row].Hero.c_str());
                                ImGui::TextUnformatted(buf);
                            } else if (column == 5) {
                                sprintf(buf, "%s", RoomInfo.PlayerB[row].Spell.c_str());
                                ImGui::TextUnformatted(buf);
                            }
                        }
                    }
                    ImGui::EndTable();
                }
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Enemy");
                if (ImGui::BeginTable("##Enemy", 6, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner)) {
                    ImGui::TableSetupColumn("Name", 0, 150);
                    ImGui::TableSetupColumn("User ID", 0, 160);
                    ImGui::TableSetupColumn("Squad", 0, 190);
                    ImGui::TableSetupColumn("Rank", 0, 180);
                    ImGui::TableSetupColumn("Hero", 0, 130);
                    ImGui::TableSetupColumn("Spell", 0, 120);
                    ImGui::TableHeadersRow();
                    for (int row = 0; row < 5; row++) {
                        ImGui::TableNextRow();
                        for (int column = 0; column < 6; column++) {
                            ImGui::TableSetColumnIndex(column);
                            char buf[32];
                            if (column == 0) {
                                sprintf(buf, "%s", RoomInfo.PlayerR[row].Name.c_str());
                                ImGui::TextUnformatted(buf);
                            } else if (column == 1) {
                                sprintf(buf, "%s", RoomInfo.PlayerR[row].UserID.c_str());
                                ImGui::TextUnformatted(buf);
                            } else if (column == 2) {
                                sprintf(buf, "%s", RoomInfo.PlayerR[row].Squad.c_str());
                                ImGui::TextUnformatted(buf);
                            } else if (column == 3) {
                                sprintf(buf, "%s", RoomInfo.PlayerR[row].Rank.c_str());
                                ImGui::TextUnformatted(buf);
                            } else if (column == 4) {
                                sprintf(buf, "%s", RoomInfo.PlayerR[row].Hero.c_str());
                                ImGui::TextUnformatted(buf);
                            } else if (column == 5) {
                                sprintf(buf, "%s", RoomInfo.PlayerR[row].Spell.c_str());
                                ImGui::TextUnformatted(buf);
                            }
                        }
                    }
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

    }
    ImGui::End(); // Must ALWAYS be called after Begin(), regardless of Begin's return value.

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (clearMousePos) {
        io->MousePos = ImVec2(-1, -1);
        clearMousePos = false;
    }
    return orig_eglSwapBuffers(dpy, surface);
}

void *main_thread(void *) {
    while (!m_IL2CPP) {
        m_IL2CPP = Tools::GetBaseAddress(m_IL2CPPLIB);
        sleep(1);
    }

    while (!m_EGL) {
        m_EGL = dlopen_ex("libEGL.so", RTLD_NOW);
        sleep(1);
    }

    Il2CppAttach(m_IL2CPPLIB);
    sleep(5);

    Tools::Hook((void *) dlsym_ex(m_EGL, "eglSwapBuffers"), (void *) _eglSwapBuffers, (void **) &orig_eglSwapBuffers);

    // Crash #1 fix: BattleManager.Update tidak ada di v2.1.88 -> offset 0
    // Dobby hook ke 0x0 menyebabkan SIGSEGV 5 detik setelah load. Guard wajib.
    { uintptr_t _bmu = BattleManager_Update;
      if (_bmu) Tools::Hook((void *) _bmu, (void *) UpdateMapHack, (void **) &orig_UpdateMapHack); }

    // Bug#2 guard: TryUseSkill punya valid offset, guard untuk safety
    { uintptr_t _suat = ShowUnitAIComp_TryUseSkill;
      if (_suat) Tools::Hook((void *) _suat, (void *) TryUseSkill, (void **) &orig_TryUseSkill); }
    // Bug#2 fix: ShowUnitAIComp.Update TIDAK ADA di v2.1.88 -> offset 0 -> Dobby crash.
    // UpdateRetribution (auto-smite) di-disable sampai ditemukan hook alternatif.
    { uintptr_t _suau = ShowUnitAIComp_Update;
      if (_suau) Tools::Hook((void *) _suau, (void *) UpdateRetribution, (void **) &orig_UpdateRetribution); }

    Tools::Hook((void *) SystemData_GetHeroSkin, (void *) GetHeroSkin, (void **) &oGetHeroSkin);
    Tools::Hook((void *) SystemData_GetMCLimitSkin, (void *) GetMCLimitSkin, (void **) &oGetMCLimitSkin);
    Tools::Hook((void *) SystemData_GetHeroHolyStatue, (void *) GetHeroHolyStatue, (void **) &oGetHeroHolyStatue);
    Tools::Hook((void *) SystemData_IsCanUseSkin, (void *) IsCanUseSkin, (void **) &oIsCanUseSkin);
    Tools::Hook((void *) SystemData_IsHaveSkin, (void *) IsHaveSkin, (void **) &oIsHaveSkin);
    Tools::Hook((void *) SystemData_IsHaveSkinForever, (void *) IsHaveSkinForever, (void **) &oIsHaveSkinForever);
    Tools::Hook((void *) SystemData_IsHaveStatue, (void *) IsHaveStatue, (void **) &oIsHaveStatue);
    Tools::Hook((void *) SystemData_IsHaveStatueForever, (void *) IsHaveStatueForever, (void **) &oIsHaveStatueForever);
    Tools::Hook((void *) SystemData_IsForbidStatue, (void *) IsForbidStatue, (void **) &oIsForbidStatue);
    Tools::Hook((void *) SystemData_IsForbidSkin, (void *) IsForbidSkin, (void **) &oIsForbidSkin);
    Tools::Hook((void *) SystemData_IsForbidARSkin, (void *) IsForbidARSkin, (void **) &oIsForbidARSkin);

    Tools::Hook((void *) UIChooseHero_SendSelectSkin, (void *) SendSelectSkin, (void **) &oSendSelectSkin);

    Tools::Hook((void *) UIRankHero_BatttleSelectSkin, (void *) BatttleSelectSkin, (void **) &oBatttleSelectSkin);

    Tools::Hook((void *) UIRankHero_ChangeShow_SendUseSkin, (void *) SendUseSkin, (void **) &oSendUseSkin);

    Tools::Hook((void *) BattleReceiveMessage_SetPlayerData, (void *) SetPlayerData, (void **) &oSetPlayerData);
    // Bug#4 fix: SetPlayerData 1-param tidak ada di v2.1.88 -> offset 0.
    { uintptr_t _spd_ = BattleReceiveMessage_SetPlayerData_;
      if (_spd_) Tools::Hook((void *) _spd_, (void *) SetPlayerData_, (void **) &oSetPlayerData_); }
    Tools::Hook((void *) BattleReceiveMessage_AddPlayerInfo, (void *) AddPlayerInfo, (void **) &oAddPlayerInfo);
    // Bug#3 fix: AddPlayerInfo 2-param tidak ada di v2.1.88 -> offset 0.
    { uintptr_t _api_ = BattleReceiveMessage_AddPlayerInfo_;
      if (_api_) Tools::Hook((void *) _api_, (void *) AddPlayerInfo_, (void **) &oAddPlayerInfo_); }

    // Bug#5 fix: GameServerConfig.SendRawData tidak ada di v2.1.88 -> offset 0.
    { uintptr_t _gsr = GameServerConfig_SendRawData;
      if (_gsr) Tools::Hook((void *) _gsr, (void *) SendRawData, (void **) &oSendRawData); }

    pthread_t t;
    return 0;
}

__attribute__((constructor))
void lib_main() {
    pthread_t ptid;
    pthread_create(&ptid, NULL, main_thread, NULL);
}

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    return JNI_VERSION_1_6;
}
