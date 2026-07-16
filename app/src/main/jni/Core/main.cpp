#include <jni.h>
#include <pthread.h>
#include <android/input.h>
#include <cstdlib>
#include <iostream>
#include <string>
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

// Guard: imgui_go TIDAK boleh pakai Il2Cpp sampai ini true
volatile bool il2cpp_ready = false;

// ============================================================
// Input hook (native level — tidak butuh Il2Cpp)
// ============================================================
#define HOOKAF(ret, func, ...) \
    ret (*orig##func)(__VA_ARGS__); \
    ret my##func(__VA_ARGS__)

HOOKAF(void, Input, void *thiz, void *ex_ab, void *ex_ac) {
    origInput(thiz, ex_ab, ex_ac);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent*)thiz);
}

// ============================================================
// ImGui setup — pakai EGL untuk ukuran layar (tidak butuh Il2Cpp)
// ============================================================
void SetupImgui(EGLDisplay dpy, EGLSurface surface) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Ambil ukuran dari EGL langsung — tidak butuh Il2Cpp sama sekali
    EGLint w = 1080, h = 1920;
    eglQuerySurface(dpy, surface, EGL_WIDTH,  &w);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &h);
    io.DisplaySize = ImVec2((float)w, (float)h);

    ImGui::StyleColorsDark();
    ImGuiStyle* style = &ImGui::GetStyle();
    style->Alpha            = 1.0f;
    style->WindowRounding   = 6.0f;
    style->FrameRounding    = 4.0f;
    style->WindowTitleAlign = ImVec2(0.5f, 0.5f);

    // Tema biru-putih FR Legends
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
    io.Fonts->AddFontFromMemoryTTF(
        &Roboto_Regular, sizeof(Roboto_Regular), 32.0f,
        &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
    ImGui::GetStyle().ScaleAllSizes(3.0f);
}

// ============================================================
// FR Legends hooks
// ============================================================

// PlayerData::GetBuySlotGold — offset 0x140a444 (arm64-v8a)
bool IsGetBuySlotGold = false;
int (*old_GetBuySlotGold)(void* instance, void* method_info);
int GetBuySlotGold(void* instance, void* method_info) {
    if (instance != nullptr && IsGetBuySlotGold)
        return 0;
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
// EGL hook — rendering pipeline, TIDAK pakai Il2Cpp sama sekali
// ============================================================
EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    static bool is_setup = false;

    if (!is_setup) {
        SetupImgui(dpy, surface);   // pakai EGL, aman dipanggil kapan saja
        is_setup = true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    DrawMenu();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::EndFrame();

    return old_eglSwapBuffers(dpy, surface);
}

// ============================================================
// Thread 1: hook EGL + input (tidak butuh Il2Cpp)
// ============================================================
void *imgui_go(void*) {
    void* handle_egl   = xdl_open("libEGL.so",  XDL_DEFAULT);
    void* handle_input = xdl_open("libinput.so", XDL_DEFAULT);

    void* sym_egl   = xdl_sym(handle_egl,   "eglSwapBuffers", nullptr);
    void* sym_input = xdl_sym(handle_input,
        "_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE",
        nullptr);

    if (sym_egl)   DobbyHook(sym_egl,   (void*)hook_eglSwapBuffers, (void**)&old_eglSwapBuffers);
    if (sym_input) DobbyHook(sym_input, (void*)myInput,             (void**)&origInput);

    pthread_exit(nullptr);
}

// ============================================================
// Thread 2: hook game functions via Il2Cpp
// ============================================================
void *hack_thread(void*) {
    // Tunggu libil2cpp.so termuat
    do {
        libBaseAddress = findLibrary(LIB);
    } while (libBaseAddress == 0);

    // Attach Il2Cpp — WAJIB sebelum panggil Il2CppGetMethodOffset
    Il2CppAttach("libil2cpp.so");
    sleep(3);

    // Tandai Il2Cpp sudah siap (dipakai thread lain jika perlu)
    il2cpp_ready = true;

    // Hook PlayerData::GetBuySlotGold
    void* offset_GetBuySlotGold = Il2CppGetMethodOffset(
        "Assembly-CSharp.dll", "", "PlayerData", "GetBuySlotGold", 0);
    if (offset_GetBuySlotGold) {
        DobbyHook(offset_GetBuySlotGold,
                  (void*)GetBuySlotGold,
                  (void**)&old_GetBuySlotGold);
    }

    pthread_exit(nullptr);
}

// ============================================================
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
