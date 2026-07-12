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
#include <cstring> // for strcmp
#include <sstream>
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


bool setup;
using namespace ImGui;

#define HOOKAF(ret, func, ...) \
    ret (*orig##func)(__VA_ARGS__); \
    ret my##func(__VA_ARGS__)

HOOKAF(void, Input, void *thiz, void *ex_ab, void *ex_ac) {
    origInput(thiz, ex_ab, ex_ac);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent*)thiz);
    return;
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
    int(*get_width)(void*);
    int(*get_height)(void*);
    get_width = (int (*)(void*)) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Screen", "get_width", 0);
    get_height = (int (*)(void*)) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Screen", "get_height", 0);
    io.DisplaySize = ImVec2((float)get_width(0), (float)get_height(0));
    
ImGui::StyleColorsDark(); // Set a theme
ImGuiStyle *style = &ImGui::GetStyle();

style->Alpha = 1.0f;
style->WindowTitleAlign = ImVec2(0.5, 0.5);
    ImGui_ImplOpenGL3_Init("#version 100");

    ImFontConfig font_cfg;
    io.Fonts->AddFontFromMemoryTTF(&Roboto_Regular, sizeof(Roboto_Regular), 32.0, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
    ImGui::GetStyle().ScaleAllSizes(3.0f);
}


bool clearMousePos = true;
struct UnityEngine_Vector2_Fields {
    float x;
    float y;
};

struct UnityEngine_Vector2_o {
    UnityEngine_Vector2_Fields fields;
};

enum TouchPhase {
    Began = 0,
    Moved = 1,
    Stationary = 2,
    Ended = 3,
    Canceled = 4
};

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

//EntityBase::get_m_CanSight
bool Isget_m_CanSight = false;
bool (*old_get_m_CanSight)(void* instance);
bool get_m_CanSight(void* instance) {
if (instance != NULL) {
if (Isget_m_CanSight) {
return true; // true/false.
}
}
return old_get_m_CanSight(instance);
}

void DrawMenu() {  
    const ImVec2 window_size = ImVec2(700, 600);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_Once);
    ImGui::Begin("IMGUI MODMENU", nullptr);
        ImGui::Checkbox("Map Hack", &Isget_m_CanSight);
    ImGui::End();  
}

EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {

    static bool is_setup = false;
    static bool should_clear_mouse_pos = false;

    if (!is_setup) {
SetupImgui();
        is_setup = true;
    }

    ImGuiIO &io = ImGui::GetIO();
    int (*TouchCount)(void*) = (int (*)(void*)) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Input", "get_touchCount", 0);
    int touchCount = TouchCount(nullptr);
    if (touchCount > 0) {
        should_clear_mouse_pos = false;
    } else {
        should_clear_mouse_pos = true;
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


void *imgui_go(void*) {
    void *handle_egl = xdl_open("libEGL.so", XDL_DEFAULT);
    void *handle_input = xdl_open("libinput.so", XDL_DEFAULT);
    
    void *xdl_sym_egl = xdl_sym(handle_egl, "eglSwapBuffers", nullptr);
    void *xdl_sym_input = xdl_sym(handle_input, "_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE", nullptr);
    
    DobbyHook(xdl_sym_egl, (void*)hook_eglSwapBuffers, (void**)&old_eglSwapBuffers);
    DobbyHook(xdl_sym_input, (void*)myInput, (void**)&origInput);
    
    pthread_exit(nullptr);
}

void *hack_thread(void*) {
    do {
libBaseAddress = findLibrary(LIB);
} while (libBaseAddress == 0);
Il2CppAttach("liblogic.so");
sleep(5);
    DobbyHook((void *)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "EntityBase", "get_m_CanSight", 0), (void *)get_m_CanSight, (void **)&old_get_m_CanSight);
    
    pthread_exit(nullptr);
}

__attribute__((constructor))
void lib_main() {
    pthread_t hacks;
    pthread_create(&hacks, NULL, imgui_go, NULL);
    pthread_create(&hacks, NULL, hack_thread, NULL);
}

extern "C" jint JNIEXPORT JNI_OnLoad(JavaVM *vm, void *key) {
    JNIEnv *env;
    vm->GetEnv((void**)&env, JNI_VERSION_1_6);
    return JNI_VERSION_1_6;
}
