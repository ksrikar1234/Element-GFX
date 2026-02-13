CONFIG += warn_off

viewer_variable += qt

#*************** HEADERS **********************************

# Viewers
HEADERS += \
    $$PWD/Renderer/include/Viewers/base_viewer.h 

contains(viewer_variable, qt) {
    HEADERS +=  $$PWD/Renderer/include/Viewers/viewer.h 
}


# Core
HEADERS += \
    $$PWD/Renderer/include/Core/gp_gui_geometry_descriptor.h \
    $$PWD/Renderer/include/Core/gp_gui_scene.h \
    $$PWD/Renderer/include/Core/gp_gui_entity_handle.h \
    $$PWD/Renderer/include/Core/gp_gui_communications.h \
    $$PWD/Renderer/include/Core/gp_gui_events.h \
    $$PWD/Renderer/include/Core/gp_gui_communications.h \
    $$PWD/Renderer/include/Core/gp_gui_camera.h \


# OpenGL 3.3 Specific
HEADERS += \    
    $$PWD/Renderer/include/Graphics_Drivers/OpenGL/OGL_3_3/gp_gui_opengl_3_3_shader.h \
    $$PWD/Renderer/include/Graphics_Drivers/OpenGL/OGL_3_3/gp_gui_opengl_3_3_vertex_array_object.h \
    $$PWD/Renderer/include/Graphics_Drivers/OpenGL/OGL_3_3/gp_gui_opengl_3_3_texture.h \
    $$PWD/Renderer/include/Graphics_Drivers/OpenGL/OGL_3_3/gp_gui_opengl_3_3_framebuffer.h \ 
    $$PWD/Renderer/include/Graphics_Drivers/OpenGL/OGL_3_3/gp_gui_opengl_3_3_render_kernel.h \
    $$PWD/Renderer/include/Graphics_Drivers/OpenGL/OGL_3_3/gp_gui_opengl_3_3_render_device.h 


#************* SOURCES **************************

# Viewers
SOURCES += \
    $$PWD/Renderer/src/Viewers/base_viewer.cpp 

contains(viewer_variable, qt) {
    !build_pass:message("Building Qt OpenGLWiget")
    SOURCES +=  $$PWD/Renderer/src/Viewers/viewer.cpp 
}

# Core
SOURCES += \
    $$PWD/Renderer/src/Core/gp_gui_geometry_descriptor.cpp \
    $$PWD/Renderer/src/Core/gp_gui_scene.cpp \
    $$PWD/Renderer/src/Core/gp_gui_entity_handle.cpp \
    $$PWD/Renderer/src/Core/gp_gui_communications.cpp \


# OpenGL 3.3 Specific
SOURCES += \    
    $$PWD/Renderer/src/Graphics_Drivers/OpenGL/OGL_3_3/gp_gui_opengl_3_3_shader.cpp \
    $$PWD/Renderer/src/Graphics_Drivers/OpenGL/OGL_3_3/gp_gui_opengl_3_3_vertex_array_object.cpp \
    $$PWD/Renderer/src/Graphics_Drivers/OpenGL/OGL_3_3/gp_gui_opengl_3_3_render_kernel.cpp \
    $$PWD/Renderer/src/Graphics_Drivers/OpenGL/OGL_3_3/gp_gui_opengl_3_3_render_device.cpp \
    $$PWD/Renderer/src/Graphics_Drivers/OpenGL/OGL_3_3/gp_gui_opengl_3_3_framebuffer.cpp \
    $$PWD/Renderer/src/Graphics_Drivers/OpenGL/OGL_3_3/gp_gui_opengl_3_3_texture.cpp 


# OpenGL 2.1 Specific
SOURCES += \    
    $$PWD/Renderer/src/Graphics_Drivers/OpenGL/OGL_2_1/gp_gui_opengl_2_1_vertex_array_object.cpp \
    $$PWD/Renderer/src/Graphics_Drivers/OpenGL/OGL_2_1/gp_gui_opengl_2_1_render_kernel.cpp \
    $$PWD/Renderer/src/Graphics_Drivers/OpenGL/OGL_2_1/gp_gui_opengl_2_1_render_device.cpp \
    $$PWD/Renderer/src/Graphics_Drivers/OpenGL/OGL_2_1/gp_gui_opengl_2_1_framebuffer.cpp 

#--------------------------------------------------------+
# renderer.h 
# (contains the necessary headery) for external usage 
#--------------------------------------------------------+
INCLUDEPATH +=  $$PWD/Imgui/include 

#--------------------------------------------------------+
# renderer.h 
# (contains the necessary headery) for external usage 
#--------------------------------------------------------+
INCLUDEPATH +=  $$PWD/Renderer/include 


#--------------------------------------------------------+
# Viewers
# For Acessing Different Types of Derived Viewers
#--------------------------------------------------------+
INCLUDEPATH +=  $$PWD/Renderer/include/Viewers


#--------------------------------------------------------+
# Scene Manager Core Components. 
# Scene, ECS, GeometryDescriptor, Communications etc..
#--------------------------------------------------------+
INCLUDEPATH +=  $$PWD/Renderer/include/Core 


#--------------------------------------------------------+
# Driver_Common_Interface
#--------------------------------------------------------+
INCLUDEPATH +=  $$PWD/Renderer/include/Graphics_Drivers/Driver_Common_Interface


#--------------------------------------------------------+
# OpenGL Drivers
#--------------------------------------------------------+
INCLUDEPATH +=  $$PWD/Renderer/include/Graphics_Drivers/OpenGL
INCLUDEPATH +=  $$PWD/Renderer/include/Graphics_Drivers/OpenGL/OGL_3_3 
INCLUDEPATH +=  $$PWD/Renderer/include/Graphics_Drivers/OpenGL/OGL_2_1
INCLUDEPATH +=  $$PWD/Renderer/include/Graphics_Drivers/OpenGL/shader_resources


#--------------------------------------------------------+
# For Basic Shapes. 
#--------------------------------------------------------+
INCLUDEPATH +=  $$PWD/Renderer/include/Geometry_Primitives 


INCLUDEPATH +=  $$PWD/Renderer/include/Graphics_Drivers/OpenSceneGraph

#--------------------------------------------------------+
# External_Libs and Common Utilities
#--------------------------------------------------------+
INCLUDEPATH +=  $$PWD/Renderer/external_dependencies 
INCLUDEPATH +=  $$PWD/Renderer/utilities

INCLUDEPATH += $$PWD/../../GridPro_Lib/external_libraries