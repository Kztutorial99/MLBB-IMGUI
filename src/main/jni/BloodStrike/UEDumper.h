#pragma once

/*
 * UEDumper.h - BloodStrike In-Game SDK Dumper
 * Bekerja dari DALAM proses game (no root needed)
 * Target: com.netease.newspike | libGame.so | UE4.23+
 * 
 * Cara pakai:
 * 1. #include "UEDumper.h" di draw.h
 * 2. Panggil UEDumper::DrawUI(drawList) di render loop
 * 3. Tap tombol "DUMP SDK" di menu
 * 4. Hasil: /sdcard/Android/data/com.netease.newspike/files/UEDump/
 */

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <android/log.h>

#define UELOG(...) __android_log_print(ANDROID_LOG_INFO, "UEDump3r", __VA_ARGS__)

// ─── Safe Memory Read (dalam proses sendiri = direct pointer) ───
template<typename T>
static inline T UE_Read(uintptr_t addr) {
    if (addr < 0x1000) return T{};
    return *reinterpret_cast<T*>(addr);
}

static inline std::string UE_ReadStr(uintptr_t addr, size_t maxLen = 256) {
    if (addr < 0x1000) return "";
    char buf[256] = {};
    strncpy(buf, reinterpret_cast<const char*>(addr), maxLen - 1);
    return std::string(buf);
}

// ─── ARM64 ADRP+ADD / ADRP+LDR Decode ───
static uintptr_t DecodeADRP_ADD(uintptr_t adrpAddr) {
    uint32_t adrp = UE_Read<uint32_t>(adrpAddr);
    uint32_t add  = UE_Read<uint32_t>(adrpAddr + 4);
    if ((adrp & 0x9F000000) != 0x90000000) return 0;
    if ((add  & 0xFF000000) != 0x91000000) return 0;
    int64_t page = ((int64_t)((adrp & 0x60000000) >> 17 | (adrp & 0x00FFFFE0) << 9) << 1) >> 1;
    uint64_t base = (adrpAddr & ~0xFFFULL) + (int64_t)(page >> 43 << 12) + ((page >> 12 & 0xFFFFF) << 12);
    uint64_t imm12 = (add >> 10) & 0xFFF;
    return (uintptr_t)(base + imm12);
}

static uintptr_t DecodeADRP_LDR(uintptr_t adrpAddr) {
    uint32_t adrp = UE_Read<uint32_t>(adrpAddr);
    uint32_t ldr  = UE_Read<uint32_t>(adrpAddr + 4);
    if ((adrp & 0x9F000000) != 0x90000000) return 0;
    int64_t page = ((int64_t)((adrp & 0x60000000) >> 17 | (adrp & 0x00FFFFE0) << 9) << 1) >> 1;
    uint64_t base = (adrpAddr & ~0xFFFULL) + (int64_t)(page >> 43 << 12) + ((page >> 12 & 0xFFFFF) << 12);
    uint64_t imm12 = ((ldr >> 10) & 0xFFF) << 3;
    return (uintptr_t)(base + imm12);
}

// ─── Pattern Scanner (scan di .text section libGame.so) ───
static uintptr_t PatternScan(uintptr_t start, uintptr_t end, const uint8_t* pattern, const char* mask) {
    size_t len = strlen(mask);
    for (uintptr_t i = start; i < end - len; i++) {
        bool found = true;
        for (size_t j = 0; j < len; j++) {
            if (mask[j] == 'x' && ((uint8_t*)i)[j] != pattern[j]) {
                found = false;
                break;
            }
        }
        if (found) return i;
    }
    return 0;
}

// ─── UE4 Memory Structures (UE 4.23+, FNamePool style) ───
namespace UE4 {
    // FNamePool offsets (NetEase UE4 — dari DeltaForce profile)
    static constexpr uint32_t FNamePool_CurrentBlock    = 0x8;
    static constexpr uint32_t FNamePool_CurrentByteCursor = 0xC;
    static constexpr uint32_t FNamePool_Blocks          = 0x40; // BlocksBit=18 → 0x40

    static constexpr uint32_t FNameEntry_Header         = 0x0;
    static constexpr uint32_t FNameEntry_String         = 0x2;
    static constexpr uint32_t FNameEntry_Stride         = 2;

    // FUObjectArray (NetEase layout, mirip DeltaForce)
    static constexpr uint32_t FUObjectArray_ObjObjects  = 0x0;
    static constexpr uint32_t TUObjectArray_NumElements = 0x4;  // sizeof(int32_t)
    static constexpr uint32_t TUObjectArray_Objects     = 0x10; // NumElements + 3*int32 = 0x10
    static constexpr uint32_t NumElementsPerChunk       = 0x10000;

    // FUObjectItem
    static constexpr uint32_t FUObjectItem_Object       = 0x0;
    static constexpr uint32_t FUObjectItem_Size         = 0x18;

    // UObject (NetEase layout)
    static constexpr uint32_t UObject_ClassPrivate      = 0x18;
    static constexpr uint32_t UObject_OuterPrivate      = 0x20;
    static constexpr uint32_t UObject_ObjectFlags       = 0x28;
    static constexpr uint32_t UObject_NamePrivate       = 0x2C; // FName
    static constexpr uint32_t UObject_InternalIndex     = 0x34;

    // FName
    static constexpr uint32_t FName_ComparisonIndex     = 0x0;
    static constexpr uint32_t FName_Number              = 0x4;
    static constexpr uint32_t FName_Size                = 0x8;

    // UField
    static constexpr uint32_t UField_Next               = 0x30;

    // UStruct
    static constexpr uint32_t UStruct_SuperStruct       = 0x50;
    static constexpr uint32_t UStruct_Children          = 0x60;
    static constexpr uint32_t UStruct_ChildProperties   = 0x78;
    static constexpr uint32_t UStruct_PropertiesSize    = 0x4C;

    // UFunction
    static constexpr uint32_t UFunction_EFunctionFlags  = 0xB8;
    static constexpr uint32_t UFunction_Func            = 0xD8;

    // FField
    static constexpr uint32_t FField_Next               = 0x20;
    static constexpr uint32_t FField_ClassPrivate       = 0x28;
    static constexpr uint32_t FField_NamePrivate        = 0x30;

    // FProperty
    static constexpr uint32_t FProperty_Offset_Internal = 0x4C;
    static constexpr uint32_t FProperty_ElementSize     = 0x44;
    static constexpr uint32_t FProperty_Size            = 0x80;
}

namespace UEDumper {

    // ─── State ───
    static std::atomic<bool> g_isDumping{false};
    static std::atomic<bool> g_dumpDone{false};
    static std::atomic<int>  g_dumpProgress{0};   // 0-100
    static std::atomic<int>  g_objectCount{0};
    static std::atomic<int>  g_classCount{0};
    static std::string       g_statusMsg = "Siap dump";
    static std::string       g_outputPath = "";

    static uintptr_t g_libBase  = 0;
    static uintptr_t g_libEnd   = 0;
    static uintptr_t g_GUObjectArray = 0;
    static uintptr_t g_GNames       = 0;

    // ─── Buat folder output ───
    static void MakeDir(const std::string& path) {
        mkdir(path.c_str(), 0777);
    }

    // ─── Baca FName string dari FNamePool ───
    static std::string GetNameFromPool(uintptr_t namePool, uint32_t comparisonIndex) {
        if (!namePool || comparisonIndex == 0) return "None";
        uint32_t chunkIdx = comparisonIndex >> 16;
        uint32_t inChunk  = comparisonIndex & 0xFFFF;

        uintptr_t blocksBase = namePool + UE4::FNamePool_Blocks;
        uintptr_t blockPtr = UE_Read<uintptr_t>(blocksBase + chunkIdx * sizeof(uintptr_t));
        if (!blockPtr) return "None";

        uintptr_t entry = blockPtr + inChunk * UE4::FNameEntry_Stride;
        uint16_t header = UE_Read<uint16_t>(entry + UE4::FNameEntry_Header);
        bool isWide   = (header >> 0) & 1;
        uint32_t len  = header >> 6;
        if (len == 0 || len > 1024) return "None";

        uintptr_t strAddr = entry + UE4::FNameEntry_String;
        if (isWide) {
            // Wide string → convert
            std::wstring ws(len, 0);
            memcpy(ws.data(), reinterpret_cast<void*>(strAddr), len * 2);
            std::string result(ws.begin(), ws.end());
            return result;
        } else {
            char buf[1025] = {};
            memcpy(buf, reinterpret_cast<void*>(strAddr), len);
            return std::string(buf);
        }
    }

    // ─── Baca FName dari UObject ───
    static std::string GetUObjectName(uintptr_t obj, uintptr_t namePool) {
        uint32_t compIdx = UE_Read<uint32_t>(obj + UE4::UObject_NamePrivate + UE4::FName_ComparisonIndex);
        uint32_t number  = UE_Read<uint32_t>(obj + UE4::UObject_NamePrivate + UE4::FName_Number);
        std::string name = GetNameFromPool(namePool, compIdx);
        if (number > 0) name += "_" + std::to_string(number - 1);
        return name;
    }

    // ─── Get UObject full path (Outer.Outer.Name) ───
    static std::string GetFullPath(uintptr_t obj, uintptr_t namePool) {
        if (!obj) return "None";
        std::string name = GetUObjectName(obj, namePool);
        uintptr_t outer = UE_Read<uintptr_t>(obj + UE4::UObject_OuterPrivate);
        if (outer && outer != obj) {
            return GetUObjectName(outer, namePool) + "." + name;
        }
        return name;
    }

    // ─── Dump satu UClass ke stream ───
    static void DumpClass(std::ostringstream& oss, uintptr_t obj, uintptr_t namePool) {
        std::string className = GetUObjectName(obj, namePool);
        uintptr_t superPtr    = UE_Read<uintptr_t>(obj + UE4::UStruct_SuperStruct);
        std::string superName = superPtr ? GetUObjectName(superPtr, namePool) : "None";
        uint32_t propSize     = UE_Read<uint32_t>(obj + UE4::UStruct_PropertiesSize);

        oss << "\n// Class: " << className
            << " | Super: " << superName
            << " | Size: 0x" << std::hex << propSize << std::dec << "\n";
        oss << "class " << className;
        if (superName != "None") oss << " : public " << superName;
        oss << " {\npublic:\n";

        // ─ Dump Properties (FField chain, UE4.25+) ─
        uintptr_t fieldPtr = UE_Read<uintptr_t>(obj + UE4::UStruct_ChildProperties);
        int propCount = 0;
        while (fieldPtr && propCount < 512) {
            uintptr_t fieldClass = UE_Read<uintptr_t>(fieldPtr + UE4::FField_ClassPrivate);
            uint32_t  fieldNameIdx = UE_Read<uint32_t>(fieldPtr + UE4::FField_NamePrivate);
            std::string propName = GetNameFromPool(namePool, fieldNameIdx);
            int32_t  offset  = UE_Read<int32_t>(fieldPtr  + UE4::FProperty_Offset_Internal);
            int32_t  elemSz  = UE_Read<int32_t>(fieldPtr  + UE4::FProperty_ElementSize);

            if (!propName.empty() && offset >= 0 && offset < 0x10000) {
                oss << "    /* 0x" << std::hex << offset << std::dec
                    << " | Sz:0x" << std::hex << elemSz << std::dec
                    << " */ auto " << propName << ";\n";
                propCount++;
            }
            fieldPtr = UE_Read<uintptr_t>(fieldPtr + UE4::FField_Next);
        }

        // ─ Dump Functions (UField children chain) ─
        uintptr_t childPtr = UE_Read<uintptr_t>(obj + UE4::UStruct_Children);
        int fnCount = 0;
        while (childPtr && fnCount < 256) {
            // Cek apakah UFunction (flag check)
            uint32_t fnFlags = UE_Read<uint32_t>(childPtr + UE4::UFunction_EFunctionFlags);
            if (fnFlags & 0x400) { // FUNC_Native
                uint32_t fnNameIdx = UE_Read<uint32_t>(childPtr + UE4::UObject_NamePrivate);
                std::string fnName = GetNameFromPool(namePool, fnNameIdx);
                uintptr_t fnPtr  = UE_Read<uintptr_t>(childPtr + UE4::UFunction_Func);
                if (!fnName.empty() && fnPtr) {
                    uintptr_t offset = fnPtr - g_libBase;
                    oss << "    // Func: " << fnName
                        << " | Offset: 0x" << std::hex << offset << std::dec << "\n";
                    fnCount++;
                }
            }
            childPtr = UE_Read<uintptr_t>(childPtr + UE4::UField_Next);
        }

        oss << "};\n";
    }

    // ─── Auto-find GUObjectArray via Pattern Scan ───
    // Pattern dari DeltaForce (NetEase family = mirip BloodStrike)
    static uintptr_t FindGUObjectArray(uintptr_t libBase, uintptr_t libEnd) {
        // Pattern 1: common NetEase UE4 pattern
        const uint8_t pat1[] = {0x91, 0xE1, 0x03, 0x00, 0xAA, 0xE0, 0x03, 0x08, 0xAA};
        const char*   msk1   = "xxx?xxxxx";
        uintptr_t hit = PatternScan(libBase, libEnd, pat1, msk1);
        if (hit) {
            uintptr_t res = DecodeADRP_ADD(hit - 0x1C);
            if (res > libBase && res < libEnd) return res;
        }

        // Pattern 2: fallback
        const uint8_t pat2[] = {0xF4, 0x03, 0x01, 0x2A, 0x00, 0x00, 0x34};
        const char*   msk2   = "xxxxxxx";
        hit = PatternScan(libBase, libEnd, pat2, msk2);
        if (hit) {
            uintptr_t res = DecodeADRP_ADD(hit + 0x18);
            if (res > libBase && res < libEnd) return res;
        }

        UELOG("GUObjectArray pattern tidak ketemu - coba manual offset");
        return 0;
    }

    // ─── Auto-find GNames (FNamePool) ───
    static uintptr_t FindGNames(uintptr_t libBase, uintptr_t libEnd) {
        // Pattern dari DeltaForce (NetEase)
        const uint8_t pat[] = {0x91, 0x00, 0x10, 0x81, 0x52, 0x00, 0x00, 0x21, 0x8B};
        const char*   msk   = "x?xxxxx?x";
        uintptr_t hit = PatternScan(libBase, libEnd, pat, msk);
        if (hit) {
            uintptr_t res = DecodeADRP_ADD(hit - 7);
            if (res > libBase && res < libEnd) return res;
        }

        UELOG("GNames pattern tidak ketemu - coba manual offset");
        return 0;
    }

    // ─── MAIN DUMP THREAD ───
    static void DumpThread() {
        g_isDumping   = true;
        g_dumpDone    = false;
        g_dumpProgress= 0;
        g_objectCount = 0;
        g_classCount  = 0;

        UELOG("=== UEDumper BloodStrike START ===");
        g_statusMsg = "Cari base libGame.so...";

        // ─ Ambil base address libGame.so ─
        g_libBase = (uintptr_t)absoluteAddress("libGame.so");
        if (!g_libBase) {
            g_statusMsg = "GAGAL: libGame.so tidak ditemukan!";
            g_isDumping = false;
            return;
        }

        // Estimasi ukuran lib (baca dari /proc/self/maps)
        g_libEnd = g_libBase + 0x10000000; // 256MB safe default
        FILE* maps = fopen("/proc/self/maps", "r");
        if (maps) {
            char line[512];
            uintptr_t maxEnd = 0;
            while (fgets(line, sizeof(line), maps)) {
                if (strstr(line, "libGame.so")) {
                    uintptr_t s, e;
                    if (sscanf(line, "%lx-%lx", &s, &e) == 2) {
                        if (e > maxEnd) maxEnd = e;
                    }
                }
            }
            fclose(maps);
            if (maxEnd > g_libBase) g_libEnd = maxEnd;
        }

        UELOG("libGame.so base: 0x%lx | end: 0x%lx", g_libBase, g_libEnd);
        g_dumpProgress = 10;
        g_statusMsg = "Cari GUObjectArray + GNames...";

        // ─ Pattern scan GUObjectArray ─
        g_GUObjectArray = FindGUObjectArray(g_libBase, g_libEnd);
        g_GNames        = FindGNames(g_libBase, g_libEnd);

        if (!g_GUObjectArray || !g_GNames) {
            g_statusMsg = "Pattern scan gagal.\nCoba update offset manual!";
            // Coba baca dari output.h offset (fallback hardcode)
            // User bisa update 2 baris ini setelah update game:
            // g_GUObjectArray = g_libBase + 0x????????;
            // g_GNames        = g_libBase + 0x????????;
            g_isDumping = false;
            return;
        }

        UELOG("GUObjectArray: 0x%lx | GNames: 0x%lx", g_GUObjectArray, g_GNames);
        g_dumpProgress = 20;
        g_statusMsg = "Scan semua UObject...";

        // ─ Buat folder output ─
        std::string outDir = "/sdcard/Android/data/" PACKAGE_NAME "/files/UEDump";
        MakeDir("/sdcard/Android/data/" PACKAGE_NAME "/files");
        MakeDir(outDir);
        g_outputPath = outDir;

        // ─ Ambil TUObjectArray ─
        uintptr_t objArray  = g_GUObjectArray + UE4::FUObjectArray_ObjObjects;
        int32_t numElements = UE_Read<int32_t>(objArray + UE4::TUObjectArray_NumElements);
        uintptr_t objsPtr   = UE_Read<uintptr_t>(objArray + UE4::TUObjectArray_Objects);

        if (numElements <= 0 || numElements > 5000000 || !objsPtr) {
            g_statusMsg = "TUObjectArray invalid!\nOffset mungkin salah.";
            g_isDumping = false;
            return;
        }

        UELOG("Total objects: %d", numElements);
        g_objectCount = numElements;
        g_dumpProgress = 30;

        // ─ Stream output files ─
        std::ofstream fObjects(outDir + "/Objects.txt");
        std::ofstream fSDK(outDir + "/AIOHeader.hpp");
        std::ofstream fOffsets(outDir + "/Offsets.txt");

        // Header AIOHeader.hpp
        fSDK << "// BloodStrike UE4 SDK Dump\n"
             << "// Generated by UEDumper (In-Game, No Root)\n"
             << "// Package: " PACKAGE_NAME "\n"
             << "// libGame.so base: 0x" << std::hex << g_libBase << std::dec << "\n"
             << "// GUObjectArray: 0x" << std::hex << (g_GUObjectArray - g_libBase) << std::dec << "\n"
             << "// GNames: 0x" << std::hex << (g_GNames - g_libBase) << std::dec << "\n\n"
             << "#pragma once\n#include <cstdint>\n\n";

        // Offsets.txt
        fOffsets << "=== BloodStrike libGame.so Offsets ===\n"
                 << "Base:          0x" << std::hex << g_libBase << "\n"
                 << "GUObjectArray: 0x" << std::hex << (g_GUObjectArray - g_libBase) << "\n"
                 << "GNames:        0x" << std::hex << (g_GNames - g_libBase) << "\n\n";

        int classesFound = 0;
        int chunksTotal = (numElements + UE4::NumElementsPerChunk - 1) / UE4::NumElementsPerChunk;
        std::ostringstream sdkStream;

        // ─ Iterasi semua chunk ─
        for (int chunk = 0; chunk < chunksTotal && chunk < 64; chunk++) {
            uintptr_t chunkPtr = UE_Read<uintptr_t>(objsPtr + chunk * sizeof(uintptr_t));
            if (!chunkPtr) continue;

            int chunkMax = std::min((int)UE4::NumElementsPerChunk,
                                    numElements - chunk * (int)UE4::NumElementsPerChunk);

            for (int i = 0; i < chunkMax; i++) {
                uintptr_t itemAddr = chunkPtr + i * UE4::FUObjectItem_Size;
                uintptr_t obj = UE_Read<uintptr_t>(itemAddr + UE4::FUObjectItem_Object);
                if (!obj || obj < 0x1000) continue;

                std::string name = GetUObjectName(obj, g_GNames);
                if (name.empty() || name == "None") continue;

                // Tulis ke Objects.txt
                uintptr_t classPtr = UE_Read<uintptr_t>(obj + UE4::UObject_ClassPrivate);
                std::string className = classPtr ? GetUObjectName(classPtr, g_GNames) : "?";
                uint32_t idx = UE_Read<uint32_t>(obj + UE4::UObject_InternalIndex);

                fObjects << "[" << std::dec << idx << "] "
                         << className << " "
                         << GetFullPath(obj, g_GNames) << "\n";

                // Dump class bila UClass/UStruct
                if (className == "Class" || className == "BlueprintGeneratedClass"
                    || className == "ScriptClass") {
                    DumpClass(sdkStream, obj, g_GNames);
                    classesFound++;
                    // Flush ke file setiap 50 class
                    if (classesFound % 50 == 0) {
                        fSDK << sdkStream.str();
                        sdkStream.str("");
                        sdkStream.clear();
                    }
                }
            }

            // Update progress (30-90%)
            g_dumpProgress = 30 + (chunk * 60 / std::max(chunksTotal, 1));
            g_statusMsg = "Dumping... chunk " + std::to_string(chunk+1)
                        + "/" + std::to_string(chunksTotal)
                        + " | " + std::to_string(classesFound) + " class";
        }

        // Flush sisa SDK
        fSDK << sdkStream.str();
        fSDK.close();
        fObjects.close();
        fOffsets.close();

        g_classCount   = classesFound;
        g_dumpProgress = 100;
        g_dumpDone     = true;
        g_isDumping    = false;
        g_statusMsg    = "SELESAI! " + std::to_string(classesFound)
                       + " class | Cek:\n" + outDir;

        UELOG("=== DUMP SELESAI: %d class, %d objects ===", classesFound, numElements);
    }

    // ─── ImGui UI — panggil di render loop draw.h ───
    static void DrawUI(ImDrawList* drawList) {
        // Warna tema
        const ImU32 colBg     = IM_COL32(15, 15, 25, 230);
        const ImU32 colBorder = IM_COL32(80, 200, 120, 255);
        const ImU32 colTitle  = IM_COL32(80, 200, 120, 255);
        const ImU32 colText   = IM_COL32(220, 220, 220, 255);
        const ImU32 colBtn    = IM_COL32(30, 140, 80, 255);
        const ImU32 colBtnHov = IM_COL32(50, 180, 100, 255);
        const ImU32 colBar    = IM_COL32(80, 200, 120, 255);
        const ImU32 colBarBg  = IM_COL32(40, 40, 60, 255);

        // Panel position
        float panelX = ImGui::GetIO().DisplaySize.x * 0.5f - 200;
        float panelY = ImGui::GetIO().DisplaySize.y * 0.5f - 120;
        float panelW = 400;
        float panelH = 240;

        ImGui::SetNextWindowPos(ImVec2(panelX, panelY), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(panelW, panelH), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.92f);

        ImGui::PushStyleColor(ImGuiCol_WindowBg,     ImVec4(0.06f, 0.06f, 0.1f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_Border,       ImVec4(0.31f, 0.78f, 0.47f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Button,       ImVec4(0.12f, 0.55f, 0.31f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(0.20f, 0.70f, 0.40f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.40f, 0.22f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

        if (ImGui::Begin("UE SDK Dumper", nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {

            // ─ Title ─
            ImGui::TextColored(ImVec4(0.31f, 0.78f, 0.47f, 1.0f),
                " BloodStrike SDK Dumper (No Root)");
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                " libGame.so | UE4.23+ | FNamePool");
            ImGui::Separator();

            // ─ Status ─
            ImGui::TextWrapped("%s", g_statusMsg.c_str());

            // ─ Progress Bar ─
            ImGui::Spacing();
            char progBuf[32];
            snprintf(progBuf, sizeof(progBuf), "%d%%", (int)g_dumpProgress);
            ImGui::ProgressBar((float)g_dumpProgress / 100.0f,
                ImVec2(-1, 18), progBuf);

            // ─ Stats ─
            if (g_objectCount > 0) {
                ImGui::Text("Objects: %d | Classes: %d",
                    (int)g_objectCount, (int)g_classCount);
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // ─ Tombol DUMP ─
            if (g_isDumping) {
                ImGui::BeginDisabled();
                ImGui::Button("  DUMPING...  ", ImVec2(-1, 36));
                ImGui::EndDisabled();
            } else if (g_dumpDone) {
                if (ImGui::Button("  DUMP ULANG  ", ImVec2(-1, 36))) {
                    g_dumpDone = false;
                    g_statusMsg = "Memulai ulang dump...";
                    std::thread(DumpThread).detach();
                }
            } else {
                if (ImGui::Button("  DUMP SDK  ", ImVec2(-1, 36))) {
                    g_statusMsg = "Memulai dump...";
                    std::thread(DumpThread).detach();
                }
            }

            // ─ Path output jika sudah selesai ─
            if (g_dumpDone && !g_outputPath.empty()) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f),
                    "Output: /sdcard/Android/data/");
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f),
                    " " PACKAGE_NAME "/files/UEDump/");
            }
        }
        ImGui::End();

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(5);
    }

    // ─── Toggle show/hide panel ───
    static bool g_showPanel = false;

    // Tambahkan ini di dalam menu utama mod kamu:
    // if (ImGui::Button("UE DUMPER")) UEDumper::g_showPanel = !UEDumper::g_showPanel;
    // if (UEDumper::g_showPanel) UEDumper::DrawUI(ImGui::GetWindowDrawList());

} // namespace UEDumper
