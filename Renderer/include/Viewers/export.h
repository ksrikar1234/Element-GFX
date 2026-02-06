#ifndef EXPORT_H
#define EXPORT_H

#if defined(BUILD_SHARED_LIB)

#if defined(_WIN32)
    #define LIB_API __declspec(dllexport)
#elif defined(_WIN32)
    #define LIB_API __declspec(dllimport)
#else
    #define LIB_API __attribute__((visibility("default")))
#endif

#else

#define LIB_API

#endif

#endif // EXPORT_H
