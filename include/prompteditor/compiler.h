#ifndef PROMPTEDITOR_COMPILER_H
#define PROMPTEDITOR_COMPILER_H

/*
 * Compiler and platform detection macros.
 *
 * This header provides a reference pattern for portable C code across the
 * four supported compiler families (MSVC, GCC, Clang, AppleClang) and three
 * target platforms (Windows, Linux, macOS).
 */

/* ---- Compiler family detection ---- */

#if defined(_MSC_VER)
#define PROMPTEDITOR_COMPILER_MSVC 1
#elif defined(__clang__) && defined(__APPLE__)
#define PROMPTEDITOR_COMPILER_APPLECLANG 1
#elif defined(__clang__)
#define PROMPTEDITOR_COMPILER_CLANG 1
#elif defined(__GNUC__)
#define PROMPTEDITOR_COMPILER_GCC 1
#else
#warning "Unknown compiler. Portable macros may not be available."
#endif

/* ---- Platform detection ---- */

#if defined(_WIN32)
#define PROMPTEDITOR_PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
#define PROMPTEDITOR_PLATFORM_MACOS 1
#elif defined(__linux__)
#define PROMPTEDITOR_PLATFORM_LINUX 1
#endif

/* ---- Portable function attributes ---- */

#if PROMPTEDITOR_COMPILER_MSVC
#define PROMPTEDITOR_NORETURN __declspec(noreturn)
#define PROMPTEDITOR_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#define PROMPTEDITOR_NORETURN __attribute__((__noreturn__))
#define PROMPTEDITOR_DEPRECATED(msg) __attribute__((__deprecated__(msg)))
#endif

#endif /* PROMPTEDITOR_COMPILER_H */
