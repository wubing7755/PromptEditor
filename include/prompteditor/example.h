#ifndef prompteditor_H
#define prompteditor_H

#include <prompteditor/compiler.h>
#include <prompteditor/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PromptEditorVersion {
    int major;
    int minor;
    int patch;
} PromptEditorVersion;

/* Returns the library version compiled into this build. */
PROMPTEDITOR_API PromptEditorVersion prompteditor_version(void);

/*
 * Adds two integers and writes the result to out_value.
 * Returns 1 on success and 0 when out_value is NULL or the sum would overflow.
 */
PROMPTEDITOR_API int prompteditor_add_checked(int left, int right, int *out_value);

/* Returns a human-readable platform name for the current target. */
PROMPTEDITOR_API const char *prompteditor_platform_name(void);

#ifdef __cplusplus
}
#endif

#endif /* prompteditor_H */
