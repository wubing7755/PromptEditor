#include <promptlib/example.h>
#include <promptlib/version.h>

#include <limits.h>

PP_Version pp_version(void) {
    PP_Version version = {
        PP_VERSION_MAJOR,
        PP_VERSION_MINOR,
        PP_VERSION_PATCH,
    };
    return version;
}

int pp_add_checked(int left, int right, int *out_value) {
    if (!out_value) {
        return 0;
    }

    if ((right > 0 && left > INT_MAX - right) || (right < 0 && left < INT_MIN - right)) {
        return 0;
    }

    *out_value = left + right;
    return 1;
}

const char *pp_platform_name(void) {
#if PP_PLATFORM_WINDOWS
    return "windows";
#elif PP_PLATFORM_MACOS
    return "macos";
#elif PP_PLATFORM_LINUX
    return "linux";
#else
    return "unknown";
#endif
}
