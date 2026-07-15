#pragma once

#define m_IL2CPPLIB "liblogic.so"
uintptr_t m_IL2CPP;
bool g_Initialized, clearMousePos = true;
int glWidth, glHeight;
void *m_EGL;

enum TouchPhase : int {
	Began = 0,
	Moved = 1,
	Stationary = 2,
	Ended = 3,
	Canceled = 4,
};

struct UnityEngine_Touch_Fields {
    int32_t m_FingerId;
    Vector2 m_Position;
    Vector2 m_RawPosition;
    Vector2 m_PositionDelta;
    float m_TimeDelta;
    int32_t m_TapCount;
    int32_t m_Phase;
    int32_t m_Type;
    float m_Pressure;
    float m_maximumPossiblePressure;
    float m_Radius;
    float m_RadiusVariance;
    float m_AltitudeAngle;
    float m_AzimuthAngle;
};

/*Class Input*/
#define Input_GetTouch (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Input", "GetTouch", 1)
#define Input_get_touchSupported (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Input", "get_touchSupported")
#define Input_get_touchCount (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Input", "get_touchCount")
#define Input_get_mousePosition (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Input", "get_mousePosition")

bool get_touchSupported(void *instance) {
	return reinterpret_cast<bool(__fastcall *)(void *)>(Input_get_touchSupported)(instance);
}

int get_touchCount() {
	return reinterpret_cast<int(__fastcall *)()>(Input_get_touchCount)();
}

Vector3 get_mousePosition(void *instance) {
	return reinterpret_cast<Vector3(__fastcall *)(void *)>(Input_get_mousePosition)(instance);
}

/*Class Screen*/
#define Screen_get_width (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Screen", "get_width")
#define Screen_get_height (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Screen", "get_height")
#define Screen_get_dpi (uintptr_t) Il2CppGetMethodOffset("UnityEngine.dll", "UnityEngine", "Screen", "get_dpi")

int get_width() {
	return reinterpret_cast<int(__fastcall *)()>(Screen_get_width)();
}

int get_height() {
	return reinterpret_cast<int(__fastcall *)()>(Screen_get_height)();
}

int get_dpi() {
	return reinterpret_cast<int(__fastcall *)()>(Screen_get_dpi)();
}

void ImGui_GetTouch(ImGuiIO* io, int screenHeight) {
    // Unity Input is a static class — instance must be nullptr for IL2CPP static calls.
    void *instance = nullptr;
    if (get_touchCount() > 0) {
        // get_touchSupported is also static; always pass nullptr.
        UnityEngine_Touch_Fields touch = ((UnityEngine_Touch_Fields (*)(void *, int))(Input_GetTouch))(instance, 0);
        Vector3 mousePos = get_mousePosition(instance);
        switch (touch.m_Phase) {
        case TouchPhase::Began:
        case TouchPhase::Stationary:
            io->MouseDown[0] = true;
            io->MousePos = ImVec2(mousePos.x, screenHeight - mousePos.y);
            break;
        case TouchPhase::Moved:
            // Keep button held during drag so ImGui can move the window.
            io->MouseDown[0] = true;
            io->MousePos = ImVec2(mousePos.x, screenHeight - mousePos.y);
            break;
        case TouchPhase::Ended:
        case TouchPhase::Canceled:
            io->MouseDown[0] = false;
            clearMousePos = true;
            break;
        default:
            break;
        }
    } else {
        // No fingers on screen — release mouse so ImGui doesn't stay stuck.
        io->MouseDown[0] = false;
    }
}
