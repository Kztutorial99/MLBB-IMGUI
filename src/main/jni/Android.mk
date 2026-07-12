LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := dobby
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libdobby.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := MEOW

LOCAL_CFLAGS := -w -s -Wno-error=format-security -fvisibility=hidden -fpermissive -fexceptions
LOCAL_CPPFLAGS := -w -s -Wno-error=format-security -fvisibility=hidden -Werror -std=c++11 -std=c++17
LOCAL_CPPFLAGS += -Wno-error=c++11-narrowing -fpermissive -Wall -fexceptions
LOCAL_LDFLAGS += -Wl,--gc-sections,--strip-all, -llog
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv3
LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES := $(LOCAL_PATH)/Core
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Modules/xdl
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Modules/ImGui
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Modules/ImGui/backends
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Modules/ImGui/font
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Modules/Hook/Dobby
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Modules/Hook/KittyMemory
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Modules/Utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Modules/Utils/Unity
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Modules/Utils/Unity/ByNameModding

FILE_LIST := $(wildcard $(LOCAL_PATH)/Core/*.cpp*)
FILE_LIST += $(wildcard $(LOCAL_PATH)/Modules/xdl/*.c*)
FILE_LIST += $(wildcard $(LOCAL_PATH)/Modules/ImGui/*.cpp*)
FILE_LIST += $(wildcard $(LOCAL_PATH)/Modules/ImGui/backends/*.cpp*)
FILE_LIST += $(wildcard $(LOCAL_PATH)/Modules/Hook/KittyMemory/*.cpp*)
FILE_LIST += $(wildcard $(LOCAL_PATH)/Modules/Utils/Unity/ByNameModding/*.cpp*)

LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_STATIC_LIBRARIES := dobby

include $(BUILD_SHARED_LIBRARY)