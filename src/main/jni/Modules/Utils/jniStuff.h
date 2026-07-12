#pragma once

JavaVM *jvm;

const char *getClipboardText() {
    const char *result;
    JNIEnv *env;
    
    jvm->AttachCurrentThread(&env, NULL);
    
    auto looperClass = env->FindClass("android/os/Looper");
    auto prepareMethod = env->GetStaticMethodID(looperClass, "prepare", "()V");
   
    jclass activityThreadClass = env->FindClass("android/app/ActivityThread");
    jfieldID sCurrentActivityThreadField = env->GetStaticFieldID(activityThreadClass, "sCurrentActivityThread", "Landroid/app/ActivityThread;");
    jobject sCurrentActivityThread = env->GetStaticObjectField(activityThreadClass, sCurrentActivityThreadField);
    
    jfieldID mInitialApplicationField = env->GetFieldID(activityThreadClass, "mInitialApplication", "Landroid/app/Application;");
    jobject mInitialApplication = env->GetObjectField(sCurrentActivityThread, mInitialApplicationField);
    
    auto contextClass = env->FindClass("android/content/Context");
    auto getSystemServiceMethod = env->GetMethodID(contextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    
    auto str = env->NewStringUTF("clipboard");
    auto clipboardManager = env->CallObjectMethod(mInitialApplication, getSystemServiceMethod, str);
  
    jclass ClipboardManagerClass = env->FindClass("android/content/ClipboardManager");
    auto getText = env->GetMethodID(ClipboardManagerClass, "getText", "()Ljava/lang/CharSequence;");

    jclass CharSequenceClass = env->FindClass("java/lang/CharSequence");
    auto toStringMethod = env->GetMethodID(CharSequenceClass, "toString", "()Ljava/lang/String;");

    auto text = env->CallObjectMethod(clipboardManager, getText);
    if (text) {
        str = (jstring) env->CallObjectMethod(text, toStringMethod);
        result = env->GetStringUTFChars(str, 0);  
    }
    return result;
}