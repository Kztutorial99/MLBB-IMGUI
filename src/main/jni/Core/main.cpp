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
// Class: Battle.LogicFighter, Assembly-CSharp.dll
// ═════════════════════════════════════════════════════════════

// ── [1] Map Hack ─────────────────────────────────────────────
// dump.cs: public virtual Boolean get_m_CanSight() — argsCount=0
// Musuh selalu terlihat di minimap
bool IsMapHack = false;
bool (*old_get_m_CanSight)(void* thiz);
bool my_get_m_CanSight(void* thiz) {
    if (thiz && IsMapHack) return true;
    return old_get_m_CanSight(thiz);
}

// ── [2] Speed Hack ────────────────────────────────────────────
// dump.cs: public Double GetMoveSpeed(Boolean bSummonOwner) — argsCount=1
// Multiply base speed dengan slider
bool IsSpeedHack = false;
float SpeedMultiplier = 2.0f;
double (*old_GetMoveSpeed)(void* thiz, bool bSummonOwner);
double my_GetMoveSpeed(void* thiz, bool bSummonOwner) {
    double base = old_GetMoveSpeed(thiz, bSummonOwner);
    if (thiz && IsSpeedHack) return base * (double)SpeedMultiplier;
    return base;
}

// ── MENU ─────────────────────────────────────────────────────
void DrawMenu() {
    ImGui::SetNextWindowSize(ImVec2(750, 450), ImGuiCond_Once);
    ImGui::Begin("MLBB MOD MENU v2.1.88", nullptr, ImGuiWindowFlags_NoCollapse);

    if (ImGui::CollapsingHeader("  MAP", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Map Hack", &IsMapHack);
        ImGui::TextDisabled("Musuh selalu kelihatan di minimap");
    }

    if (ImGui::CollapsingHeader("  MOVEMENT", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Speed Hack", &IsSpeedHack);
        if (IsSpeedHack) {
            ImGui::SliderFloat("Speed x", &SpeedMultiplier, 1.0f, 5.0f);
        }
        ImGui::TextDisabled("Multiply move speed");
    }

    ImGui::Separator();
    ImGui::TextDisabled("v1 — 2 fitur stabil");
    ImGui::TextDisabled("Fitur tambahan menyusul setelah tes ini lulus");

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

    // Input via Unity Touch API — koordinat akurat + Y-flip
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
// IMGUI THREAD — EGL hook saja
// ═════════════════════════════════════════════════════════════
void* imgui_go(void*) {
    void* h   = xdl_open("libEGL.so", XDL_DEFAULT);
    void* sym = xdl_sym(h, "eglSwapBuffers", nullptr);
    DobbyHook(sym, (void*)hook_eglSwapBuffers, (void**)&old_eglSwapBuffers);
    pthread_exit(nullptr);
}

// ═════════════════════════════════════════════════════════════
// HACK THREAD — Il2Cpp game hooks
// sleep(5) → beri waktu game init sebelum hook dipasang
// ═════════════════════════════════════════════════════════════
void* hack_thread(void*) {
    do { libBaseAddress = findLibrary(LIB); } while (!libBaseAddress);
    Il2CppAttach("liblogic.so");
    sleep(5);

    // [1] Map Hack — Battle.LogicFighter::get_m_CanSight, argsCount=0
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "get_m_CanSight", 0),
        (void*)my_get_m_CanSight, (void**)&old_get_m_CanSight
    );

    // [2] Speed Hack — Battle.LogicFighter::GetMoveSpeed, argsCount=1
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "GetMoveSpeed", 1),
        (void*)my_GetMoveSpeed, (void**)&old_GetMoveSpeed
    );

    // ── FITUR BARU DINONAKTIFKAN SEMENTARA ────────────────────
    // Aktifkan SATU PER SATU setelah 2 fitur di atas terbukti stabil.
    // [3] Attack Range  — GetAttackRange,    argsCount=0, return Double
    // [4] Attack Speed  — get_m_AtkSpeed,    argsCount=0, return Double
    // [5] No Cooldown   — GetCoolPer,        argsCount=0, return Double
    // [6] No Mana Cost  — GetMpCostPer,      argsCount=0, return Double
    // [7] Cant Be Atk   — get_m_bDontBeAtk,  argsCount=0, return Boolean
    // ─────────────────────────────────────────────────────────

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
