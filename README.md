# MLBB ImGui — Mobile Legends Bang Bang ImGui Mod

> ⚠️ **Untuk Agent / Contributor Baru — Baca Ini Dulu!**

## 🔧 Build Environment

Project ini **dibangun menggunakan [AIDE PRO](https://www.android-ide.com/)** langsung dari **HP Android** — bukan PC, bukan Android Studio, bukan terminal Linux.

### Workflow Build:
1. `git clone https://github.com/Kztutorial99/MLBB-IMGUI.git` di AIDE PRO
2. Buka project di AIDE PRO
3. Build langsung dari HP Android

## ⚠️ Penting untuk Agent / Developer

| ❌ Jangan | ✅ Lakukan |
|-----------|-----------|
| Sarankan build via terminal/CLI | Sesuaikan dengan AIDE PRO build system |
| Sarankan Android Studio | Gunakan AIDE PRO |
| Ubah struktur folder sembarangan | Konfirmasi dulu sebelum ubah struktur |
| Sarankan Gradle dari PC | Build system sudah diatur untuk AIDE PRO |

---

## 🎮 Fitur Mod Menu

| Fitur | Kategori | Status |
|-------|----------|--------|
| Map Hack | Map | ✅ |
| God Mode | Battle | ✅ |
| No Cooldown | Battle | ✅ |
| Speed Hack | Movement | ✅ |

---

## 🔩 Arsitektur Hook (untuk Agent)

Semua fitur menggunakan **ByNameModding IL2CPP framework** + **Dobby hook**.

### Pola wajib setiap fitur baru:

```cpp
// 1. Global toggle + original pointer
bool IsFeature = false;
ReturnType (*old_Method)(void* thiz, args...);

// 2. Hook function
ReturnType my_Method(void* thiz, args...) {
    if (thiz != NULL && IsFeature) { return modifiedValue; }
    return old_Method(thiz, args...);  // selalu panggil original jika off
}

// 3. Checkbox di DrawMenu()
ImGui::Checkbox("Label", &IsFeature);

// 4. DobbyHook di hack_thread — SETELAH Il2CppAttach + sleep(5)
DobbyHook(
    (void*)Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "ClassName", "Method", argsCount),
    (void*)my_Method,
    (void**)&old_Method
);
```

### ⚠️ Aturan Kritis

- **Semua game hook HANYA di `hack_thread`** — setelah `Il2CppAttach("liblogic.so")` + `sleep(5)`
- **`Il2CppGetMethodOffset` game TIDAK BOLEH** dipanggil di `imgui_go` atau `SetupImgui`
- **`argsCount`** = jumlah parameter di dump.cs, **tidak termasuk** `this`
- **Return type** harus match persis dengan dump.cs
- **Selalu panggil `old*()`** ketika fitur dimatikan

### Referensi dump.cs

File: `DATA/com.mobile.legends_2.1.88.12027_1783893858261.cs`  
Game version: **2.1.88.12027**

```
// Implementasi saat ini:
public virtual Boolean  get_m_CanSight();                                       // argsCount=0
public Double           GetMoveSpeed(Boolean bSummonOwner);                     // argsCount=1
public virtual Void     BeAtkModifyHP(Int32 value, Battle.LogicFighter pAtk);   // argsCount=2
public Int32            CalcSkillCoolDown(Int32 iCoolDownTime, Int32 iSpellId); // argsCount=2
```

---

## 📁 Struktur Project

```
MLBB-IMGUI/
├── build.gradle
├── src/main/jni/
│   ├── Core/
│   │   ├── main.cpp          ← file utama mod menu
│   │   └── Include.h
│   └── Modules/
│       ├── Hook/Dobby/       ← Dobby hooking library
│       ├── ImGui/            ← Dear ImGui
│       ├── Utils/Unity/ByNameModding/  ← Il2Cpp helper
│       └── xdl/              ← xdl dynamic linking
├── DATA/
│   └── *.cs                  ← IL2CPP dump (referensi offset)
└── build/bin/
    └── *.apk
```

---

*Repo dikelola via Replit Agent — edit kode di Replit, push ke GitHub, user git pull di AIDE PRO.*
