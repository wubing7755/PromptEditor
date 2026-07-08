#ifndef PP_EXAMPLE_H
#define PP_EXAMPLE_H

#include <prompteditor/compiler.h>
#include <prompteditor/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PP_Version {
    int major;
    int minor;
    int patch;
} PP_Version;

/** Returns the library version compiled into this build. */
PP_API PP_Version pp_version(void);

/**
 * Adds two integers and writes the result to out_value.
 *
 * @param left      First operand.
 * @param right     Second operand.
 * @param out_value Pointer to result storage; must not be NULL.
 * @return 1 on success, 0 on overflow or NULL out_value.
 */
PP_API int pp_add_checked(int left, int right, int *out_value);

/** Returns a human-readable platform name for the current target. */
PP_API const char *pp_platform_name(void);

#ifdef __cplusplus
}
#endif

#endif /* PP_EXAMPLE_H */
