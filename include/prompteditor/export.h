#ifndef PROMPTEDITOR_EXPORT_H
#define PROMPTEDITOR_EXPORT_H

#if defined(_WIN32) && defined(PROMPTEDITOR_SHARED)
#if defined(PROMPTEDITOR_BUILDING_LIBRARY)
#define PROMPTEDITOR_API __declspec(dllexport)
#else
#define PROMPTEDITOR_API __declspec(dllimport)
#endif
#else
#define PROMPTEDITOR_API
#endif

#endif /* PROMPTEDITOR_EXPORT_H */
