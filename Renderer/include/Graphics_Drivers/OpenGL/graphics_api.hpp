#ifndef __GRIDPRO_OPENGL_RENDERER_API_ENTRY__
#define __GRIDPRO_OPENGL_RENDERER_API_ENTRY__

#define GP_ENABLE_OPENGL_2_1_QT_DRIVER

#include <stdexcept>

class QOpenGLFunctions_4_1_Core;
class QOpenGLFunctions_4_1_Compatibility;
class QOpenGLFunctions_4_3_Core;
class QOpenGLFunctions_2_1;
class GLEW_OpenGLFunctions_4_1_Core;

#ifdef USE_GLEW_OPENGL_API_ENTRY
// Use GLEW OpenGL API

#if defined(__gl_h_) || defined(__GL_H__) || defined(_GL_H) || defined(__X_GL_H)
#error gl.h included before glew.h
#endif

#include "glew_opengl_api.hpp"

#define GP_ENABLE_OPENGL_4_1_GLEW_DRIVER

#define GP_DEFAULT_OPENGL_DRIVER_API GLEW_OpenGLFunctions_4_1_Core
#define GP_CURRENT_OPENGL_DRIVER_API GLEW_OpenGLFunctions_4_1_Core

#endif  
// Use Qt OpenGL API

#ifndef USE_GLEW_OPENGL_API_ENTRY

#ifdef  Q_OS_MAC

#define GP_ENABLE_OPENGL_4_1_QT_DRIVER
#define GP_DEFAULT_OPENGL_DRIVER_API QOpenGLFunctions_4_1_Core
#define GP_CURRENT_OPENGL_DRIVER_API QOpenGLFunctions_4_1_Core
#else

#define GP_ENABLE_OPENGL_4_3_QT_DRIVER 

#define GP_DEFAULT_OPENGL_DRIVER_API QOpenGLFunctions_4_1_Compatibility
#define GP_CURRENT_OPENGL_DRIVER_API QOpenGLFunctions_4_1_Compatibility

#endif

#endif


#ifdef GP_ENABLE_OPENGL_4_1_QT_DRIVER
#include <QOpenGLFunctions_4_1_Core>
#endif

#ifdef GP_ENABLE_OPENGL_4_3_QT_DRIVER
#include <QOpenGLFunctions_4_1_Compatibility>
#include <QOpenGLFunctions_4_3_Core>
#endif

#include <QOpenGLFunctions_2_1>


class __GraphicsAPI__
{
public:

    static __GraphicsAPI__* instance() 
    {
        static __GraphicsAPI__ instance;
        return &instance;
    }

/// @brief Overloaded operators to return the OpenGL functions object
/// @return OpenGL functions object
/// @note These operators are used to implicitly cast the RendererAPI instance to the correct OpenGL functions object

    operator QOpenGLFunctions_4_1_Core* () 
    {
        #ifdef GP_ENABLE_OPENGL_4_1_QT_DRIVER
          return &OpenGL_4_1_Corefunctions;
        #else
            throw std::runtime_error("OpenGL 4.1 driver not enabled");
        #endif
    }

    operator QOpenGLFunctions_4_1_Compatibility*() 
    {
        #ifdef GP_ENABLE_OPENGL_4_3_QT_DRIVER
          return &OpenGL_4_3_functions;
        #else
          throw std::runtime_error("OpenGL 4.3 Compatibility driver not enabled");
        #endif

    }
    operator QOpenGLFunctions_4_3_Core*() 
    {
        #ifdef GP_ENABLE_OPENGL_4_3_QT_DRIVER
          return &OpenGL_4_3_Corefunctions;
        #else
          throw std::runtime_error("OpenGL 4.3 Core driver not enabled");
        #endif
    }

    operator GLEW_OpenGLFunctions_4_1_Core*() 
    {
        #ifdef GP_ENABLE_OPENGL_4_1_GLEW_DRIVER
          return &GLEW_OpenGLFunctions_4_1_CoreFunctions;
        #else
          throw std::runtime_error("OpenGL 4.1 driver not enabled");
        #endif
    }

    operator QOpenGLFunctions_2_1* () 
    {
        #ifdef GP_ENABLE_OPENGL_2_1_QT_DRIVER
          return &OpenGL_2_1_functions;
        #else
          printf("OpenGL 2.1 driver not enabled\n");
          return nullptr;
        #endif  
    }

private:
    __GraphicsAPI__() = default;
   ~__GraphicsAPI__() = default;

    __GraphicsAPI__(const __GraphicsAPI__&) = delete;
    __GraphicsAPI__& operator=(const __GraphicsAPI__&) = delete;

    __GraphicsAPI__(__GraphicsAPI__&&) = delete;
    __GraphicsAPI__& operator=(__GraphicsAPI__&&) = delete;
    
private :

#ifdef GP_ENABLE_OPENGL_4_1_QT_DRIVER
    QOpenGLFunctions_4_1_Core OpenGL_4_1_Corefunctions;
#endif

#ifdef GP_ENABLE_OPENGL_4_3_QT_DRIVER
    QOpenGLFunctions_4_1_Compatibility OpenGL_4_3_functions;
    QOpenGLFunctions_4_3_Core OpenGL_4_3_Corefunctions;
#endif

#ifdef GP_ENABLE_OPENGL_4_1_GLEW_DRIVER
    GLEW_OpenGLFunctions_4_1_Core GLEW_OpenGLFunctions_4_1_CoreFunctions;
#endif

  #ifdef GP_ENABLE_OPENGL_2_1_QT_DRIVER
    QOpenGLFunctions_2_1 OpenGL_2_1_functions;
  #endif
};

#ifdef GP_ENABLE_OPENGL_4_1_GLEW_DRIVER

using QGL_4_1       = GLEW_OpenGLFunctions_4_1_Core;
using QGL_3_3       = GLEW_OpenGLFunctions_4_1_Core;

#endif


#ifdef GP_ENABLE_OPENGL_4_1_QT_DRIVER

using QGL_4_1       = QOpenGLFunctions_4_1_Core;
using QGL_3_3       = QOpenGLFunctions_4_1_Core;

#endif

#ifdef GP_ENABLE_OPENGL_4_3_QT_DRIVER

using QGL_3_3       = QOpenGLFunctions_4_1_Compatibility;
using QGL_3_3_Core  = QOpenGLFunctions_4_3_Core;

#endif

using QGL_2_1 = QOpenGLFunctions_2_1;

using QGL_DEFAULT = GP_DEFAULT_OPENGL_DRIVER_API;
using QGL_CURRENT = GP_CURRENT_OPENGL_DRIVER_API;

#ifdef GP_ENABLE_OPENGL_2_1_QT_DRIVER
using CURR_QGL = QGL_2_1;
#else
using CURR_QGL = QGL_DEFAULT;
#endif

template <typename GL_API_TYPE = CURR_QGL>
inline GL_API_TYPE* RendererAPI() 
{
    static_assert(
        std::is_same<GL_API_TYPE, QOpenGLFunctions_4_1_Core>::value ||
        std::is_same<GL_API_TYPE, QOpenGLFunctions_4_1_Compatibility>::value ||
        std::is_same<GL_API_TYPE, QOpenGLFunctions_4_3_Core>::value||
        std::is_same<GL_API_TYPE, GLEW_OpenGLFunctions_4_1_Core>::value ||
        std::is_same<GL_API_TYPE, QOpenGLFunctions_2_1>::value,
        "Unsupported OpenGL API type"
    );
    
    return *(__GraphicsAPI__::instance());
}

#endif // __GRIDPRO_OPENGL_RENDERER_API_ENTRY__