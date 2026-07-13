#include <jni.h>
#include <pthread.h>
#include <android/input.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cstring>
#include <array>
#include <unistd.h>
#include <sys/system_properties.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>

#include "xdl.h"
#include "dobby.h"
#include "Include.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_android.h"
#include "Roboto-Regular.h"
#include "Icon.h"
#include "Iconcpp.h"

using namespace ImGui;

// ── Unity Touch Structs ───────────────────────────────────────
struct UnityEngine_Vector2_Fields { float x; float y; };
struct UnityEngine_Vector2_o { UnityEngine_Vector2_Fields fields; };
enum TouchPhase { Began=0, Moved=1, Stationary=2, Ended=3, Canceled=4 };
struct UnityEngine_Touch_Fields {
    int32_t m_FingerId;
    UnityEngine_Vector2_o m_Position;
    UnityEngine_Vector2_o m_RawPosition;
    UnityEngine_Vector2_o m_PositionDelta;
    float m_TimeDelta;
    int32_t m_TapCount;
    int32_t m_Phase;
    int32_t m_Type;
    float m_Pressure;
    float m_maximumPossiblePressure;
    float m_Radius;
    float m_fRadiusVariance;
    float m_AltitudeAngle;
    float m_AzimuthAngle;
};

// ── Safe DobbyHook ────────────────────────────────────────────
static void SafeHook(void* addr, void* replace, void** origin) {
    if (!addr) return;
    DobbyHook(addr, replace, origin);
}

// ═════════════════════════════════════════════════════════════
// FITUR — dikonfirmasi dari dump.cs v2.1.88.12027
// Class: Battle.LogicFighter | Assembly: Assembly-CSharp.dll
//
// FIX KRITIS: il2cpp ABI selalu tambahkan hidden parameter
// "MethodInfo*" di akhir setiap method signature.
// Contoh nyata dari il2cpp guide:
//   C#: double get_m_AtkSpeed()
//   C++: double get_m_AtkSpeed(void* thiz, void* method_info)
// Tanpa method_info → stack corruption → FC saat masuk ingame
// ═════════════════════════════════════════════════════════════

// ── [1] Map Hack ─────────────────────────────────────────────
// dump.cs line 550212: public virtual Boolean get_m_CanSight()
// argsCount=0 — SIGNATURE BENAR: (thiz, method_info)
bool IsMapHack = false;
bool (*old_get_m_CanSight)(void* thiz, void* method_info);
bool my_get_m_CanSight(void* thiz, void* method_info) {
    if (thiz && IsMapHack) return true;
    return old_get_m_CanSight(thiz, method_info);
}

// ── [2] Speed Hack ────────────────────────────────────────────
// dump.cs line 550303: public Double CalcRealSpeed(Int32 value)
// argsCount=1 — SIGNATURE BENAR: (thiz, value, method_info)
bool IsSpeedHack = false;
float SpeedMultiplier = 2.0f;
double (*old_CalcRealSpeed)(void* thiz, int32_t value, void* method_info);
double my_CalcRealSpeed(void* thiz, int32_t value, void* method_info) {
    double base = old_CalcRealSpeed(thiz, value, method_info);
    if (thiz && IsSpeedHack) return base * (double)SpeedMultiplier;
    return base;
}

// ── [3] Attack Range Hack ─────────────────────────────────────
// dump.cs line 556311: public virtual Double GetAttackRange()
// argsCount=0 — SIGNATURE BENAR: (thiz, method_info)
bool IsAttackRange = false;
float RangeMultiplier = 3.0f;
double (*old_GetAttackRange)(void* thiz, void* method_info);
double my_GetAttackRange(void* thiz, void* method_info) {
    double base = old_GetAttackRange(thiz, method_info);
    if (thiz && IsAttackRange) return base * (double)RangeMultiplier;
    return base;
}

// ── [4] Attack Speed Hack ─────────────────────────────────────
// dump.cs line 550262: public virtual Double get_m_AtkSpeed()
// argsCount=0 — SIGNATURE BENAR: (thiz, method_info)
bool IsAtkSpeed = false;
float AtkSpeedMult = 3.0f;
double (*old_get_m_AtkSpeed)(void* thiz, void* method_info);
double my_get_m_AtkSpeed(void* thiz, void* method_info) {
    double base = old_get_m_AtkSpeed(thiz, method_info);
    if (thiz && IsAtkSpeed) return base * (double)AtkSpeedMult;
    return base;
}

// ── [5] No Cooldown ───────────────────────────────────────────
// dump.cs line 550316: public Double GetCoolPer()
// argsCount=0 — SIGNATURE BENAR: (thiz, method_info)
bool IsNoCooldown = false;
double (*old_GetCoolPer)(void* thiz, void* method_info);
double my_GetCoolPer(void* thiz, void* method_info) {
    if (thiz && IsNoCooldown) return 0.0;
    return old_GetCoolPer(thiz, method_info);
}

// ── [6] No Mana Cost ──────────────────────────────────────────
// dump.cs line 550313: public Double GetMpCostPer()
// argsCount=0 — SIGNATURE BENAR: (thiz, method_info)
bool IsNoMana = false;
double (*old_GetMpCostPer)(void* thiz, void* method_info);
double my_GetMpCostPer(void* thiz, void* method_info) {
    if (thiz && IsNoMana) return 0.0;
    return old_GetMpCostPer(thiz, method_info);
}

// ── [7] Can't Be Attacked ─────────────────────────────────────
// dump.cs line 550220: public Boolean get_m_bDontBeAtk()
// argsCount=0 — SIGNATURE BENAR: (thiz, method_info)
bool IsDontBeAtk = false;
bool (*old_get_m_bDontBeAtk)(void* thiz, void* method_info);
bool my_get_m_bDontBeAtk(void* thiz, void* method_info) {
    if (thiz && IsDontBeAtk) return true;
    return old_get_m_bDontBeAtk(thiz, method_info);
}

// ═════════════════════════════════════════════════════════════
// MENU
// ═════════════════════════════════════════════════════════════
void DrawMenu() {
    ImGui::SetNextWindowSize(ImVec2(780, 860), ImGuiCond_Once);
    ImGui::Begin("MLBB MOD MENU v2.1.88", nullptr, ImGuiWindowFlags_NoCollapse);

    // MAP
    if (ImGui::CollapsingHeader("  MAP", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Map Hack", &IsMapHack);
    }

    // MOVEMENT
    if (ImGui::CollapsingHeader("  MOVEMENT", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Speed Hack", &IsSpeedHack);
        if (IsSpeedHack) {
            ImGui::SliderFloat("Speed x", &SpeedMultiplier, 1.0f, 5.0f);
        }
    }

    // COMBAT
    if (ImGui::CollapsingHeader("  COMBAT", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Attack Range", &IsAttackRange);
        if (IsAttackRange) {
            ImGui::SliderFloat("Range x", &RangeMultiplier, 1.0f, 8.0f);
        }
        ImGui::Separator();
        ImGui::Checkbox("Attack Speed", &IsAtkSpeed);
        if (IsAtkSpeed) {
            ImGui::SliderFloat("Atk Speed x", &AtkSpeedMult, 1.0f, 5.0f);
        }
        ImGui::Separator();
        ImGui::Checkbox("No Cooldown", &IsNoCooldown);
        ImGui::Separator();
        ImGui::Checkbox("No Mana Cost", &IsNoMana);
    }

    // DEFENSE
    if (ImGui::CollapsingHeader("  DEFENSE", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Can't Be Attacked", &IsDontBeAtk);
    }

    ImGui::End();
}

// ═════════════════════════════════════════════════════════════
// IMGUI SETUP
// ═════════════════════════════════════════════════════════════
void SetupImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    int (*get_width)(void*)  = (int(*)(void*)) Il2CppGetMethodOffset(
        "UnityEngine.dll", "UnityEngine", "Screen", "get_width",  0);
    int (*get_height)(void*) = (int(*)(void*)) Il2CppGetMethodOffset(
        "UnityEngine.dll", "UnityEngine", "Screen", "get_height", 0);
    if (get_width && get_height) {
        io.DisplaySize = ImVec2((float)get_width(nullptr), (float)get_height(nullptr));
    }

    ImGui::StyleColorsDark();
    ImGuiStyle* s = &ImGui::GetStyle();
    s->Alpha            = 1.0f;
    s->WindowTitleAlign = ImVec2(0.5f, 0.5f);
    s->WindowRounding   = 8.0f;
    s->FrameRounding    = 5.0f;
    s->GrabRounding     = 5.0f;

    ImGui_ImplOpenGL3_Init("#version 100");

    ImFontConfig fc;
    io.Fonts->AddFontFromMemoryTTF(
        &Roboto_Regular, sizeof(Roboto_Regular), 30.0f,
        &fc, io.Fonts->GetGlyphRangesCyrillic());
    ImGui::GetStyle().ScaleAllSizes(3.0f);
}

// ═════════════════════════════════════════════════════════════
// EGL SWAP BUFFERS — Render + Input (Unity Touch)
// ═════════════════════════════════════════════════════════════
EGLBoolean (*old_eglSwapBuffers)(EGLDisplay, EGLSurface);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {

    static bool is_setup = false;
    if (!is_setup) {
        SetupImgui();
        is_setup = true;
    }

    static int (*fn_TouchCount)(void*) = nullptr;
    static UnityEngine_Touch_Fields (*fn_GetTouch)(void*, int32_t) = nullptr;
    if (!fn_TouchCount) {
        fn_TouchCount = (int(*)(void*)) Il2CppGetMethodOffset(
            "UnityEngine.dll", "UnityEngine", "Input", "get_touchCount", 0);
        fn_GetTouch = (UnityEngine_Touch_Fields(*)(void*, int32_t)) Il2CppGetMethodOffset(
            "UnityEngine.dll", "UnityEngine", "Input", "GetTouch", 1);
    }

    ImGuiIO& io = ImGui::GetIO();
    if (fn_TouchCount && fn_GetTouch) {
        int tc = fn_TouchCount(nullptr);
        if (tc > 0) {
            UnityEngine_Touch_Fields t = fn_GetTouch(nullptr, 0);
            float fy = io.DisplaySize.y - t.m_Position.fields.y;
            io.MousePos     = ImVec2(t.m_Position.fields.x, fy);
            io.MouseDown[0] = (t.m_Phase == Began     ||
                               t.m_Phase == Moved     ||
                               t.m_Phase == Stationary);
        } else {
            io.MouseDown[0] = false;
            io.MousePos     = ImVec2(-1.0f, -1.0f);
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    DrawMenu();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return old_eglSwapBuffers(dpy, surface);
}

// ═════════════════════════════════════════════════════════════
// IMGUI THREAD
// ═════════════════════════════════════════════════════════════
void* imgui_go(void*) {
    void* h   = xdl_open("libEGL.so", XDL_DEFAULT);
    void* sym = xdl_sym(h, "eglSwapBuffers", nullptr);
    DobbyHook(sym, (void*)hook_eglSwapBuffers, (void**)&old_eglSwapBuffers);
    pthread_exit(nullptr);
}

// ═════════════════════════════════════════════════════════════
// HACK THREAD — semua 7 fitur aktif
// sleep(10): tunggu game selesai loading
// FIX: semua hook pakai signature il2cpp yang benar (+ method_info)
// ═════════════════════════════════════════════════════════════
void* hack_thread(void*) {
    do { libBaseAddress = findLibrary(LIB); } while (!libBaseAddress);
    Il2CppAttach("liblogic.so");
    sleep(10);

    // [1] Map Hack — get_m_CanSight, argsCount=0
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "get_m_CanSight", 0),
        (void*)my_get_m_CanSight, (void**)&old_get_m_CanSight
    );

    // [2] Speed Hack — CalcRealSpeed(Int32), argsCount=1
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "CalcRealSpeed", 1),
        (void*)my_CalcRealSpeed, (void**)&old_CalcRealSpeed
    );

    // [3] Attack Range — GetAttackRange, argsCount=0
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "GetAttackRange", 0),
        (void*)my_GetAttackRange, (void**)&old_GetAttackRange
    );

    // [4] Attack Speed — get_m_AtkSpeed, argsCount=0
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "get_m_AtkSpeed", 0),
        (void*)my_get_m_AtkSpeed, (void**)&old_get_m_AtkSpeed
    );

    // [5] No Cooldown — GetCoolPer, argsCount=0
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "GetCoolPer", 0),
        (void*)my_GetCoolPer, (void**)&old_GetCoolPer
    );

    // [6] No Mana Cost — GetMpCostPer, argsCount=0
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "GetMpCostPer", 0),
        (void*)my_GetMpCostPer, (void**)&old_GetMpCostPer
    );

    // [7] Can't Be Attacked — get_m_bDontBeAtk, argsCount=0
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "get_m_bDontBeAtk", 0),
        (void*)my_get_m_bDontBeAtk, (void**)&old_get_m_bDontBeAtk
    );

    pthread_exit(nullptr);
}

// ═════════════════════════════════════════════════════════════
// ENTRY POINTS
// ═════════════════════════════════════════════════════════════
__attribute__((constructor))
void lib_main() {
    pthread_t t1, t2;
    pthread_create(&t1, nullptr, imgui_go,    nullptr);
    pthread_create(&t2, nullptr, hack_thread, nullptr);
}

extern "C" jint JNIEXPORT JNI_OnLoad(JavaVM* vm, void* key) {
    JNIEnv* env;
    vm->GetEnv((void**)&env, JNI_VERSION_1_6);
    return JNI_VERSION_1_6;
}
