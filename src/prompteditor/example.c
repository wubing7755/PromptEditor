#include <prompteditor/example.h>
#include <prompteditor/version.h>

#include <limits.h>

PromptEditorVersion prompteditor_version(void) {
    PromptEditorVersion version = {
        PROMPTEDITOR_VERSION_MAJOR,
        PROMPTEDITOR_VERSION_MINOR,
        PROMPTEDITOR_VERSION_PATCH,
    };
    return version;
}

int prompteditor_add_checked(int left, int right, int *out_value) {
    if (!out_value) {
        return 0;
    }

    if ((right > 0 && left > INT_MAX - right) || (right < 0 && left < INT_MIN - right)) {
        return 0;
    }

    *out_value = left + right;
    return 1;
}

const char *prompteditor_platform_name(void) {
#if PROMPTEDITOR_PLATFORM_WINDOWS
    return "windows";
#elif PROMPTEDITOR_PLATFORM_MACOS
    return "macos";
#elif PROMPTEDITOR_PLATFORM_LINUX
    return "linux";
#else
    return "unknown";
#endif
}
