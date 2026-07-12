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
    struct UnityEngine_Vector2_o m_Position;
    struct UnityEngine_Vector2_o m_RawPosition;
    struct UnityEngine_Vector2_o m_PositionDelta;
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

// ── Safe DobbyHook Helper ─────────────────────────────────────
// Mencegah hook ke alamat 0 (null) yang bisa menyebabkan FC
static void SafeHook(void* addr, void* replace, void** origin) {
    if (addr == nullptr || addr == (void*)0) return;
    DobbyHook(addr, replace, origin);
}

// ── [1] Map Hack ─────────────────────────────────────────────
// dump.cs: EntityBase.get_m_CanSight() : Boolean — argsCount=0
bool IsMapHack = false;
bool (*old_get_m_CanSight)(void* thiz);
bool my_get_m_CanSight(void* thiz) {
    if (thiz != nullptr && IsMapHack) return true;
    return old_get_m_CanSight(thiz);
}

// ── [2] Speed Hack ────────────────────────────────────────────
// dump.cs: LogicFighter.GetMoveSpeed(Boolean bSummonOwner) : Double — argsCount=1
bool IsSpeedHack = false;
float SpeedMultiplier = 2.5f;
double (*old_GetMoveSpeed)(void* thiz, bool bSummonOwner);
double my_GetMoveSpeed(void* thiz, bool bSummonOwner) {
    if (thiz != nullptr && IsSpeedHack) {
        double base = old_GetMoveSpeed(thiz, bSummonOwner);
        return base * (double)SpeedMultiplier;
    }
    return old_GetMoveSpeed(thiz, bSummonOwner);
}

// ── [3] Attack Range Hack ─────────────────────────────────────
// dump.cs: LogicFighter.GetAttackRange() : Double — argsCount=0
bool IsAttackRange = false;
float AttackRangeMultiplier = 3.0f;
double (*old_GetAttackRange)(void* thiz);
double my_GetAttackRange(void* thiz) {
    if (thiz != nullptr && IsAttackRange) {
        return old_GetAttackRange(thiz) * (double)AttackRangeMultiplier;
    }
    return old_GetAttackRange(thiz);
}

// ── [4] Attack Speed Hack ─────────────────────────────────────
// dump.cs: LogicFighter.GetBasicAtkCd() : Double — argsCount=0
// Return nilai kecil = cooldown pendek = serangan makin cepat
bool IsAttackSpeed = false;
double (*old_GetBasicAtkCd)(void* thiz);
double my_GetBasicAtkCd(void* thiz) {
    if (thiz != nullptr && IsAttackSpeed) {
        return 0.01; // ~instant basic attack cooldown
    }
    return old_GetBasicAtkCd(thiz);
}

// ── [5] Drone View (Camera Zoom) ──────────────────────────────
// dump.cs: BaseCameraLogic.GetMaxDistance() : Double — argsCount=0
bool IsDroneView = false;
float DroneDistance = 2.5f;
double (*old_GetMaxDistance)(void* thiz);
double my_GetMaxDistance(void* thiz) {
    if (thiz != nullptr && IsDroneView) {
        return old_GetMaxDistance(thiz) * (double)DroneDistance;
    }
    return old_GetMaxDistance(thiz);
}

// ── [6] No Mana Cost ──────────────────────────────────────────
// dump.cs: LogicFighter.GetSkillMp(Int32 skillIndex) : Double — argsCount=1
// Return 0 = skill tidak butuh mana
bool IsNoMana = false;
double (*old_GetSkillMp)(void* thiz, int32_t skillIndex);
double my_GetSkillMp(void* thiz, int32_t skillIndex) {
    if (thiz != nullptr && IsNoMana) return 0.0;
    return old_GetSkillMp(thiz, skillIndex);
}

// ── [7] Anti AFK ─────────────────────────────────────────────
// dump.cs: EntityBase.get_IsAFK() : Boolean — argsCount=0
// Return false = karakter tidak dianggap AFK
bool IsAntiAFK = false;
bool (*old_get_IsAFK)(void* thiz);
bool my_get_IsAFK(void* thiz) {
    if (thiz != nullptr && IsAntiAFK) return false;
    return old_get_IsAFK(thiz);
}

// ── MENU ─────────────────────────────────────────────────────
void DrawMenu() {
    ImGui::SetNextWindowSize(ImVec2(750, 750), ImGuiCond_Once);
    ImGui::Begin("MLBB MOD MENU", nullptr,
        ImGuiWindowFlags_NoCollapse);

    // ── Map ──
    if (ImGui::CollapsingHeader("  Map", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Map Hack", &IsMapHack);
    }

    // ── Movement ──
    if (ImGui::CollapsingHeader("  Movement", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Speed Hack", &IsSpeedHack);
        if (IsSpeedHack) {
            ImGui::SliderFloat("Speed Multiplier", &SpeedMultiplier, 1.0f, 5.0f);
        }
    }

    // ── Combat ──
    if (ImGui::CollapsingHeader("  Combat", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Attack Range Hack", &IsAttackRange);
        if (IsAttackRange) {
            ImGui::SliderFloat("Range Multiplier", &AttackRangeMultiplier, 1.0f, 6.0f);
        }
        ImGui::Separator();
        ImGui::Checkbox("Attack Speed Hack", &IsAttackSpeed);
        ImGui::Separator();
        ImGui::Checkbox("No Mana Cost", &IsNoMana);
    }

    // ── Camera ──
    if (ImGui::CollapsingHeader("  Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Drone View", &IsDroneView);
        if (IsDroneView) {
            ImGui::SliderFloat("Drone Distance", &DroneDistance, 1.0f, 5.0f);
        }
    }

    // ── Misc ──
    if (ImGui::CollapsingHeader("  Misc", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Anti AFK", &IsAntiAFK);
    }

    ImGui::End();
}

// ── SETUP IMGUI ──────────────────────────────────────────────
void SetupImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Ambil resolusi layar dari Unity
    int (*get_width)(void*)  = (int(*)(void*)) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Screen", "get_width",  0);
    int (*get_height)(void*) = (int(*)(void*)) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Screen", "get_height", 0);
    if (get_width && get_height) {
        io.DisplaySize = ImVec2((float)get_width(nullptr), (float)get_height(nullptr));
    }

    ImGui::StyleColorsDark();
    ImGuiStyle* style = &ImGui::GetStyle();
    style->Alpha            = 1.0f;
    style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style->WindowRounding   = 8.0f;
    style->FrameRounding    = 5.0f;
    style->GrabRounding     = 5.0f;

    ImGui_ImplOpenGL3_Init("#version 100");

    ImFontConfig font_cfg;
    io.Fonts->AddFontFromMemoryTTF(
        &Roboto_Regular, sizeof(Roboto_Regular), 30.0f,
        &font_cfg, io.Fonts->GetGlyphRangesCyrillic()
    );
    ImGui::GetStyle().ScaleAllSizes(3.0f);
}

// ── EGL SWAP BUFFERS (Render Loop) ───────────────────────────
EGLBoolean (*old_eglSwapBuffers)(EGLDisplay, EGLSurface);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {

    static bool is_setup = false;
    if (!is_setup) {
        SetupImgui();
        is_setup = true;
    }

    // ── Input via Unity Touch API ─────────────────────────────
    // FIX: Tidak pakai AInputEvent casting yang salah.
    // Gunakan Unity Input.GetTouch() langsung — koordinat sudah
    // dalam ruang layar yang sama dengan ImGui DisplaySize.
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
        int touchCount = fn_TouchCount(nullptr);
        if (touchCount > 0) {
            UnityEngine_Touch_Fields touch = fn_GetTouch(nullptr, 0);
            // Unity Y=0 di bawah layar, ImGui Y=0 di atas → flip
            float fy = io.DisplaySize.y - touch.m_Position.fields.y;
            io.MousePos = ImVec2(touch.m_Position.fields.x, fy);
            io.MouseDown[0] = (touch.m_Phase == Began    ||
                               touch.m_Phase == Moved    ||
                               touch.m_Phase == Stationary);
        } else {
            io.MouseDown[0] = false;
            io.MousePos     = ImVec2(-1.0f, -1.0f);
        }
    }

    // ── Render ───────────────────────────────────────────────
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    DrawMenu();
    ImGui::Render();                                   // implicitly calls EndFrame()
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // JANGAN panggil ImGui::EndFrame() setelah Render() — sudah otomatis

    return old_eglSwapBuffers(dpy, surface);
}

// ── IMGUI THREAD (EGL hook only, input sudah pakai Unity Touch) ──
void* imgui_go(void*) {
    void* handle_egl  = xdl_open("libEGL.so", XDL_DEFAULT);
    void* sym_egl     = xdl_sym(handle_egl, "eglSwapBuffers", nullptr);
    DobbyHook(sym_egl, (void*)hook_eglSwapBuffers, (void**)&old_eglSwapBuffers);
    pthread_exit(nullptr);
}

// ── HACK THREAD (semua game hook di sini) ────────────────────
void* hack_thread(void*) {
    do { libBaseAddress = findLibrary(LIB); } while (libBaseAddress == 0);
    Il2CppAttach("liblogic.so");
    sleep(5);

    // [1] Map Hack
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "EntityBase", "get_m_CanSight", 0),
        (void*)my_get_m_CanSight,
        (void**)&old_get_m_CanSight
    );

    // [2] Speed Hack
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "GetMoveSpeed", 1),
        (void*)my_GetMoveSpeed,
        (void**)&old_GetMoveSpeed
    );

    // [3] Attack Range Hack
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "GetAttackRange", 0),
        (void*)my_GetAttackRange,
        (void**)&old_GetAttackRange
    );

    // [4] Attack Speed Hack
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "GetBasicAtkCd", 0),
        (void*)my_GetBasicAtkCd,
        (void**)&old_GetBasicAtkCd
    );

    // [5] Drone View
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "BaseCameraLogic", "GetMaxDistance", 0),
        (void*)my_GetMaxDistance,
        (void**)&old_GetMaxDistance
    );

    // [6] No Mana Cost
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "GetSkillMp", 1),
        (void*)my_GetSkillMp,
        (void**)&old_GetSkillMp
    );

    // [7] Anti AFK
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "EntityBase", "get_IsAFK", 0),
        (void*)my_get_IsAFK,
        (void**)&old_get_IsAFK
    );

    pthread_exit(nullptr);
}

// ── ENTRY POINTS ─────────────────────────────────────────────
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
