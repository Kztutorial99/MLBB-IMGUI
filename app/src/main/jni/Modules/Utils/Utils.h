#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <limits.h>
#include <inttypes.h>
#include <vector>
#include <sstream>

#include "dobby.h"

uintptr_t libBase = 0;
uintptr_t libBaseAddress = 0;
uintptr_t libBaseEndAddress = 0;

#if defined(__aarch64__)
#define SplitApk "split_config.arm64_v8a.apk"
#else
#define SplitApk "split_config.armeabi_v7a.apk"
#endif

uintptr_t findLibrary(const char *library) {
    char filename[0xFF] = {0};
    char buffer[1024] = {0};

    FILE *fp = nullptr;
    uintptr_t address = 0;
    uintptr_t start = 0;
    uintptr_t end = 0;

    char flags[7];
    char path[PATH_MAX];

    sprintf(filename, "/proc/%d/maps", getpid());

    fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        return 0;
    }

    while (fgets(buffer, sizeof(buffer), fp)) {
        path[0] = '\0';

        sscanf(buffer,
               "%" PRIxPTR "-%" PRIxPTR " %6s %*x %*x:%*x %*u %s",
               &start, &end, flags, path);

        if (!strstr(flags, "r-xp"))
            continue;

        if (strstr(path, library)) {
            address = start;
            break;
        }
    }

    fclose(fp);
    return address;
}

bool isLibraryLoaded(const char *libraryName) {
    char line[512];

    FILE *fp = fopen("/proc/self/maps", "rt");
    if (!fp)
        return false;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, libraryName)) {
            fclose(fp);
            return true;
        }
    }

    fclose(fp);
    return false;
}

uintptr_t getAbsoluteAddress(const char *libraryName, uintptr_t relativeAddr) {
    libBase = findLibrary(libraryName);

    if (!libBase)
        return 0;

    return libBase + relativeAddr;
}

#define LIB "liblogic.so"

#define gAA getAbsoluteAddress

#define DHOOK(offset, hook, orig) \
    DobbyHook((void *)gAA(LIB, offset), (void *)hook, (void **)&orig)

#define MHOOK(address, hook, orig) \
    DobbyHook((void *)(address), (void *)hook, (void **)&orig)