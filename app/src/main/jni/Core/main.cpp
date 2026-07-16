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

// ============================================================
// FR Legends — com.fengiiley.frlegends
// Dump  : 64-bit | Jul 13 2026
// ABI   : arm64-v8a
// ============================================================

#define HOOKAF(ret, func, ...) \
    ret (*orig##func)(__VA_ARGS__); \
    ret my##func(__VA_ARGS__)

HOOKAF(void, Input, void *thiz, void *ex_ab, void *ex_ac) {
    origInput(thiz, ex_ab, ex_ac);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent*)thiz);
}

std::string GetProp(const char* key) {
    char value[PROP_VALUE_MAX];
    __system_property_get(key, value);
    return std::string(value);
}

void SetupImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    int(*get_width)(void*)  = (int(*)(void*)) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Screen", "get_width",  0);
    int(*get_height)(void*) = (int(*)(void*)) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Screen", "get_height", 0);
    io.DisplaySize = ImVec2((float)get_width(nullptr), (float)get_height(nullptr));

    ImGui::StyleColorsDark();
    ImGuiStyle* style = &ImGui::GetStyle();
    style->Alpha           = 1.0f;
    style->WindowRounding  = 6.0f;
    style->FrameRounding   = 4.0f;
    style->WindowTitleAlign = ImVec2(0.5f, 0.5f);

    // Warna tema FR Legends (biru-putih)
    ImVec4* c = style->Colors;
    c[ImGuiCol_WindowBg]       = ImVec4(0.08f, 0.10f, 0.15f, 0.96f);
    c[ImGuiCol_TitleBg]        = ImVec4(0.05f, 0.12f, 0.28f, 1.00f);
    c[ImGuiCol_TitleBgActive]  = ImVec4(0.07f, 0.18f, 0.40f, 1.00f);
    c[ImGuiCol_FrameBg]        = ImVec4(0.10f, 0.14f, 0.22f, 1.00f);
    c[ImGuiCol_CheckMark]      = ImVec4(0.20f, 0.60f, 1.00f, 1.00f);
    c[ImGuiCol_Button]         = ImVec4(0.07f, 0.18f, 0.40f, 1.00f);
    c[ImGuiCol_ButtonHovered]  = ImVec4(0.10f, 0.28f, 0.60f, 1.00f);
    c[ImGuiCol_ButtonActive]   = ImVec4(0.20f, 0.60f, 1.00f, 1.00f);
    c[ImGuiCol_Header]         = ImVec4(0.10f, 0.28f, 0.55f, 1.00f);
    c[ImGuiCol_HeaderHovered]  = ImVec4(0.15f, 0.38f, 0.70f, 1.00f);
    c[ImGuiCol_Separator]      = ImVec4(0.20f, 0.60f, 1.00f, 0.40f);

    ImGui_ImplOpenGL3_Init("#version 100");

    ImFontConfig font_cfg;
    io.Fonts->AddFontFromMemoryTTF(&Roboto_Regular, sizeof(Roboto_Regular), 32.0f, &font_cfg,
                                   io.Fonts->GetGlyphRangesCyrillic());
    ImGui::GetStyle().ScaleAllSizes(3.0f);
}

// ============================================================
// FR Legends — Hooks
// ============================================================

// PlayerData::GetBuySlotGold
// Offset: 0x140a444  (arm64-v8a, Assembly-CSharp.dll, no namespace)
// Signature: int(void* instance, void* method_info)
bool IsGetBuySlotGold = false;
int (*old_GetBuySlotGold)(void* instance, void* method_info);
int GetBuySlotGold(void* instance, void* method_info) {
    if (instance != nullptr && IsGetBuySlotGold) {
        return 0;
    }
    return old_GetBuySlotGold(instance, method_info);
}

// ============================================================
// Menu
// ============================================================

void DrawMenu() {
    const ImVec2 window_size = ImVec2(700, 600);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_Once);
    ImGui::Begin("FR LEGENDS MOD MENU", nullptr);

    if (ImGui::BeginTabBar("FRL_Tabs")) {

        // ---- Tab: Player ----
        if (ImGui::BeginTabItem("Player")) {
            ImGui::Spacing();
            ImGui::Text("[ Player Data ]");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Checkbox("Buy Car Slot Gold = 0", &IsGetBuySlotGold);
            ImGui::Spacing();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

// ============================================================
// EGL + Touch
// ============================================================

struct UnityEngine_Vector2_Fields { float x, y; };
struct UnityEngine_Vector2_o { UnityEngine_Vector2_Fields fields; };
enum TouchPhase { Began=0, Moved=1, Stationary=2, Ended=3, Canceled=4 };
struct UnityEngine_Touch_Fields {
    int32_t m_FingerId;
    UnityEngine_Vector2_o m_Position, m_RawPosition, m_PositionDelta;
    float m_TimeDelta;
    int32_t m_TapCount, m_Phase, m_Type;
    float m_Pressure, m_maximumPossiblePressure, m_Radius, m_fRadiusVariance, m_AltitudeAngle, m_AzimuthAngle;
};

EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    static bool is_setup = false;
    static bool should_clear_mouse_pos = false;

    if (!is_setup) {
        SetupImgui();
        is_setup = true;
    }

    ImGuiIO& io = ImGui::GetIO();
    int(*TouchCount)(void*) = (int(*)(void*)) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Input", "get_touchCount", 0);
    int touchCount = TouchCount(nullptr);

    if (touchCount > 0) {
        void*(*GetTouch)(void*, int32_t) = (void*(*)(void*, int32_t))
            Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Input", "GetTouch", 1);
        auto touch = (UnityEngine_Touch_Fields*) GetTouch(nullptr, 0);
        if (touch) {
            ImVec2 pos(touch->m_Position.fields.x,
                       io.DisplaySize.y - touch->m_Position.fields.y);
            io.MousePos = pos;
            if (touch->m_Phase == Began || touch->m_Phase == Moved || touch->m_Phase == Stationary) {
                io.MouseDown[0] = true;
            } else {
                io.MouseDown[0] = false;
                should_clear_mouse_pos = true;
            }
        }
    } else {
        io.MouseDown[0] = false;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    DrawMenu();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::EndFrame();

    if (should_clear_mouse_pos) {
        io.MousePos = ImVec2(-1, -1);
        should_clear_mouse_pos = false;
    }
    return old_eglSwapBuffers(dpy, surface);
}

// ============================================================
// Threads
// ============================================================

void *imgui_go(void*) {
    void* handle_egl   = xdl_open("libEGL.so",   XDL_DEFAULT);
    void* handle_input = xdl_open("libinput.so",  XDL_DEFAULT);

    void* sym_egl   = xdl_sym(handle_egl,   "eglSwapBuffers", nullptr);
    void* sym_input = xdl_sym(handle_input,
        "_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE",
        nullptr);

    DobbyHook(sym_egl,   (void*)hook_eglSwapBuffers, (void**)&old_eglSwapBuffers);
    DobbyHook(sym_input, (void*)myInput,             (void**)&origInput);
    pthread_exit(nullptr);
}

void *hack_thread(void*) {
    // Tunggu libil2cpp.so FR Legends termuat
    do {
        libBaseAddress = findLibrary(LIB);
    } while (libBaseAddress == 0);

    Il2CppAttach("libil2cpp.so");
    sleep(5);

    // ---- Daftarkan hook FR Legends ----
    // PlayerData::GetBuySlotGold — offset 0x140a444 (arm64-v8a)
    DobbyHook(
        (void*) Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "PlayerData", "GetBuySlotGold", 0),
        (void*) GetBuySlotGold,
        (void**) &old_GetBuySlotGold
    );

    pthread_exit(nullptr);
}

__attribute__((constructor))
void lib_main() {
    pthread_t t;
    pthread_create(&t, nullptr, imgui_go,    nullptr);
    pthread_create(&t, nullptr, hack_thread, nullptr);
}

extern "C" jint JNIEXPORT JNI_OnLoad(JavaVM* vm, void* key) {
    JNIEnv* env;
    vm->GetEnv((void**)&env, JNI_VERSION_1_6);
    return JNI_VERSION_1_6;
}
