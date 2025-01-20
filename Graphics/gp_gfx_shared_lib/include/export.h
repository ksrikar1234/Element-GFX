#ifndef EXPORT_H
#define EXPORT_H

// This Header is Only Used when Building a Shared Library  (So we Predefine BUILD_SHARED_LIB) But in the case of GridPro_GUI/Graphics/Renderer/include/export.h, BUILD_SHARED_LIB is not pre-defined 
// Because there we can build a shared library or a static library
// So we need to define BUILD_SHARED_LIB in the qmake project file

#define BUILD_SHARED_LIB

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
