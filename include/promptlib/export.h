#ifndef PP_EXPORT_H
#define PP_EXPORT_H

#if defined(_WIN32) && defined(PP_SHARED)
#if defined(PP_BUILDING_LIBRARY)
#define PP_API __declspec(dllexport)
#else
#define PP_API __declspec(dllimport)
#endif
#else
#define PP_API
#endif

#endif /* PP_EXPORT_H */
