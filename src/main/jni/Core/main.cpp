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
// FITUR
// Class: Battle.LogicFighter | Assembly-CSharp.dll
// WAJIB: semua il2cpp hook punya hidden param MethodInfo* di akhir
// Signature BENAR: (void* thiz, void* method_info)
// ═════════════════════════════════════════════════════════════

// ── [1] Map Hack ──────────────────────────────────────────────
// dump.cs line 550212
// public virtual Boolean get_m_CanSight()
// argsCount=0 | return bool
bool IsMapHack = false;
bool (*old_get_m_CanSight)(void* thiz, void* method_info);
bool my_get_m_CanSight(void* thiz, void* method_info) {
    if (thiz && IsMapHack) return true;
    return old_get_m_CanSight(thiz, method_info);
}

// ── [2] Invisible / Stealth ───────────────────────────────────
// dump.cs line 550216
// public Boolean get_m_bSkillHide()
// argsCount=0 | return bool
// Effect: hero masuk mode stealth (masuk semak) terus-menerus
//         musuh tidak bisa target/lihat hero
bool IsInvisible = false;
bool (*old_get_m_bSkillHide)(void* thiz, void* method_info);
bool my_get_m_bSkillHide(void* thiz, void* method_info) {
    if (thiz && IsInvisible) return true;
    return old_get_m_bSkillHide(thiz, method_info);
}

// ── [3] Fake Death ───────────────────────────────────────────
// dump.cs line 550222
// public Boolean get_m_bFakeDeath()
// argsCount=0 | return bool
// Effect: game anggap hero sudah mati (pasif tertentu seperti Uranus/Esmeralda)
//         musuh tidak bisa target, tapi hero masih bisa bergerak
bool IsFakeDeath = false;
bool (*old_get_m_bFakeDeath)(void* thiz, void* method_info);
bool my_get_m_bFakeDeath(void* thiz, void* method_info) {
    if (thiz && IsFakeDeath) return true;
    return old_get_m_bFakeDeath(thiz, method_info);
}

// ═════════════════════════════════════════════════════════════
// MENU
// ═════════════════════════════════════════════════════════════
void DrawMenu() {
    ImGui::SetNextWindowSize(ImVec2(780, 380), ImGuiCond_Once);
    ImGui::Begin("MLBB MOD MENU v2.1.88", nullptr, ImGuiWindowFlags_NoCollapse);

    // ── MAP ──────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("  MAP", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Map Hack", &IsMapHack);
        ImGui::TextDisabled("  Lihat semua musuh di minimap");
    }

    ImGui::Spacing();

    // ── HERO ─────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("  HERO", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Invisible (Skill Hide)", &IsInvisible);
        ImGui::TextDisabled("  Hero masuk stealth, musuh tidak bisa target");

        ImGui::Spacing();

        ImGui::Checkbox("Fake Death", &IsFakeDeath);
        ImGui::TextDisabled("  Hero tampak mati, musuh tidak bisa serang");
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
// HACK THREAD
// ═════════════════════════════════════════════════════════════
void* hack_thread(void*) {
    do { libBaseAddress = findLibrary(LIB); } while (!libBaseAddress);
    Il2CppAttach("liblogic.so");
    sleep(10);

    // [1] Map Hack — get_m_CanSight, argsCount=0, bool
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "get_m_CanSight", 0),
        (void*)my_get_m_CanSight, (void**)&old_get_m_CanSight
    );

    // [2] Invisible — get_m_bSkillHide, argsCount=0, bool
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "get_m_bSkillHide", 0),
        (void*)my_get_m_bSkillHide, (void**)&old_get_m_bSkillHide
    );

    // [3] Fake Death — get_m_bFakeDeath, argsCount=0, bool
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "get_m_bFakeDeath", 0),
        (void*)my_get_m_bFakeDeath, (void**)&old_get_m_bFakeDeath
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
