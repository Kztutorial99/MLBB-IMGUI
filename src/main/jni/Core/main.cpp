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

// ── Local Player Detection ────────────────────────────────────
// dump.cs: public virtual Boolean get_bIsMainEntity() // 0xffffffff8b04a384
// Digunakan untuk filter hook agar hanya berlaku pada hero lokal
// Tidak di-hook — hanya dipanggil sebagai function pointer
static bool (*fn_IsMainEntity)(void* thiz, void* method_info) = nullptr;

// ═════════════════════════════════════════════════════════════
// FITUR
// Class: Battle.LogicFighter | Assembly-CSharp.dll
// WAJIB: semua il2cpp hook punya hidden param MethodInfo* di akhir
// Signature BENAR: (void* thiz, void* method_info)
// ═════════════════════════════════════════════════════════════

// ── [1] Map Hack ──────────────────────────────────────────────
// dump.cs line 550212
// public virtual Boolean get_m_CanSight()    offset: 0xffffffff8bc9d0ec
// argsCount=0 | return bool | virtual — dipanggil renderer minimap
// Berlaku untuk SEMUA entity (intentional: tampilkan semua musuh di minimap)
bool IsMapHack = false;
bool (*old_get_m_CanSight)(void* thiz, void* method_info);
bool my_get_m_CanSight(void* thiz, void* method_info) {
    if (thiz && IsMapHack) return true;
    return old_get_m_CanSight(thiz, method_info);
}

// ── [2] Invisible ─────────────────────────────────────────────
// FIX: Hook InSkillHide() BUKAN get_m_bSkillHide()
//
// MENGAPA get_m_bSkillHide() TIDAK BEKERJA:
//   get_m_bSkillHide() = raw field getter (hanya return m_bSkillHide field)
//   Il2cpp compiled code membaca field m_bSkillHide LANGSUNG dari memory offset,
//   bukan melalui property getter — sehingga hook pada getter diabaikan.
//
// MENGAPA InSkillHide() BEKERJA:
//   InSkillHide() adalah method yang dipanggil oleh engine, AI musuh,
//   dan targeting system untuk cek apakah fighter dalam kondisi stealth.
//   Ini adalah entry point yang SELALU dipanggil sebelum musuh bisa target hero.
//
// dump.cs line ~550211
// public Boolean InSkillHide()    offset: 0xffffffff8bc9d0c4
// argsCount=0 | return bool
//
// FILTER LOCAL PLAYER: fn_IsMainEntity(thiz) — hanya berlaku hero sendiri
// (mencegah semua entity ikut stealth → game logic conflict)
bool IsInvisible = false;
bool (*old_InSkillHide)(void* thiz, void* method_info);
bool my_InSkillHide(void* thiz, void* method_info) {
    if (thiz && IsInvisible) {
        // Filter: hanya local player (get_bIsMainEntity == true)
        if (fn_IsMainEntity && fn_IsMainEntity(thiz, method_info)) {
            return true;
        }
    }
    return old_InSkillHide(thiz, method_info);
}

// ── [3] Cant Be Attacked ─────────────────────────────────────
// MENGGANTIKAN Fake Death yang tidak bisa diimplementasi client-side.
//
// MENGAPA Fake Death TIDAK BEKERJA:
//   m_bFakeDeath adalah flag yang DI-SET OLEH SERVER setiap game tick.
//   Hook get_m_bFakeDeath() memang return true, tapi server terus push false
//   via netcode (ISHOW_UpdateShowEye) dan override nilai setiap sync tick.
//   Fake Death murni server-controlled di MLBB multiplayer — tidak bisa
//   di-override hanya dari client-side getter hook.
//
// SOLUSI — get_m_bDontBeAtk():
//   Flag ini dicek oleh targeting system (AI musuh + skill targeted) sebelum
//   memilih target. Jika true, hero TIDAK BISA dijadikan target serangan.
//   Lebih efektif karena targeting check adalah client-side logic.
//
// dump.cs line 550220 (dari area 550216)
// public Boolean get_m_bDontBeAtk()    offset: 0xffffffff8bc9debc
// argsCount=0 | return bool
//
// FILTER LOCAL PLAYER: fn_IsMainEntity(thiz)
bool IsCantBeAtk = false;
bool (*old_get_m_bDontBeAtk)(void* thiz, void* method_info);
bool my_get_m_bDontBeAtk(void* thiz, void* method_info) {
    if (thiz && IsCantBeAtk) {
        if (fn_IsMainEntity && fn_IsMainEntity(thiz, method_info)) {
            return true;
        }
    }
    return old_get_m_bDontBeAtk(thiz, method_info);
}

// ═════════════════════════════════════════════════════════════
// MENU
// ═════════════════════════════════════════════════════════════
void DrawMenu() {
    ImGui::SetNextWindowSize(ImVec2(780, 420), ImGuiCond_Once);
    ImGui::Begin("MLBB MOD MENU v2.1.88", nullptr, ImGuiWindowFlags_NoCollapse);

    // ── MAP ──────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("  MAP", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Map Hack", &IsMapHack);
        ImGui::TextDisabled("  Lihat semua musuh di minimap");
    }

    ImGui::Spacing();

    // ── HERO ─────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("  HERO", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Invisible (InSkillHide)", &IsInvisible);
        ImGui::TextDisabled("  Hero masuk stealth — musuh tidak bisa target");

        ImGui::Spacing();

        ImGui::Checkbox("Cant Be Attacked", &IsCantBeAtk);
        ImGui::TextDisabled("  Musuh tidak bisa menyerang hero (m_bDontBeAtk)");
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

    // Resolve local player detector (tidak di-hook, hanya dipanggil)
    // dump.cs: public virtual Boolean get_bIsMainEntity() argsCount=0
    fn_IsMainEntity = (bool(*)(void*, void*)) Il2CppGetMethodOffset(
        "Assembly-CSharp.dll", "Battle", "LogicEntityBase", "get_bIsMainEntity", 0);

    // [1] Map Hack — get_m_CanSight (virtual), argsCount=0
    // Berlaku semua entity (intentional)
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "get_m_CanSight", 0),
        (void*)my_get_m_CanSight, (void**)&old_get_m_CanSight
    );

    // [2] Invisible — InSkillHide() BUKAN get_m_bSkillHide()
    // FIX: InSkillHide() adalah method yg dipanggil engine/AI untuk cek stealth
    // get_m_bSkillHide() hanya raw getter — il2cpp bypass via direct field read
    // dump.cs: public Boolean InSkillHide() argsCount=0
    SafeHook(
        (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "InSkillHide", 0),
        (void*)my_InSkillHide, (void**)&old_InSkillHide
    );

    // [3] Cant Be Attacked — get_m_bDontBeAtk()
    // FIX: Menggantikan Fake Death yang server-controlled
    // get_m_bDontBeAtk() dicek targeting system sebelum musuh bisa attack
    // dump.cs: public Boolean get_m_bDontBeAtk() argsCount=0
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
