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

## 📁 Struktur Project

```
MLBB-IMGUI/
├── build.gradle          # Gradle config (AIDE PRO compatible)
├── src/
│   └── main/
│       ├── cpp/          # Source C++ ImGui
│       ├── java/         # Java/Android source
│       ├── obj/          # Compiled objects
│       └── res/          # Android resources
└── build/
    └── bin/
        └── *.apk         # Output APK
```

## 📱 Requirements

- AIDE PRO (Android IDE) terinstall di HP Android
- NDK support aktif di AIDE PRO
- Clone repo ini langsung dari AIDE PRO

---

*Repo ini dikelola via Replit Agent + push GitHub API/git.*
