#include <iostream>

#include "gp_gui_opengl_2_1_render_device.h"
#include "gp_gui_opengl_2_1_render_kernel.h"

#include "gp_gui_communications.h"

#include "gp_gui_opengl_2_1_framebuffer.h"

#include "graphics_api.hpp"

#include "gp_gui_debug.h"

#ifndef USE_GLEW_OPENGL_API_ENTRY

#include <QOpenGLContext>
#include <QSurfaceFormat>

#endif

namespace GridPro_GFX {

using namespace OpenGL_2_1;

void OpenGL_2_1_RenderDevice::init()
{
    GP_TRACE("OpenGL_2_1_RenderDevice::init");

    #ifndef USE_GLEW_OPENGL_API_ENTRY

    QOpenGLContext* context = QOpenGLContext::currentContext();
    try
    {
    if (context) 
    {
        std::cout << "Current OpenGL context: " << context->format().version().first << "." << context->format().version().second << "\n";

        if(context->format().profile() != QSurfaceFormat::NoProfile)
        {
            GP_COLOR_PRINT(GP_COLOR::BRIGHT_GREEN, "Profile    : ");
            std::cout << (context->format().profile() == QSurfaceFormat::CoreProfile ? "Core" : "Compatibility") << "\n";
        }
        else 
            std::cout << "Profile: No Profile" << "\n";  
        
        int major_version = context->format().version().first;
        int minor_version = context->format().version().second;

        bool is_compatibility_profile = context->format().profile() == QSurfaceFormat::CompatibilityProfile;
        #ifdef Q_OS_MAC
        is_compatibility_profile = (context->format().profile() == QSurfaceFormat::NoProfile);
        #endif


        if(major_version < 2 || (major_version == 2 && minor_version < 1) || !(is_compatibility_profile))
        {
            std::cerr << "OpenGL version 2.1 or higher is required\n";
            throw std::runtime_error("OpenGL version 2.1 Compatibility or higher is required !! \n  Removing OpenGL_2_1_RenderDevice from the system\n");
        }
    }
    else
    {
        std::cout << "No current OpenGL context\n";
    }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Caught exception: " << e.what() << "\n";
        throw e;
    }

    #endif // #ifndef USE_GLEW_OPENGL_API_ENTRY
    RendererAPI<QGL_2_1>()->initializeOpenGLFunctions();

    Event::Publisher::GetInstance()->get_scene_state().m_driver_enum = SceneState::DriverEnum::OpenGL_2_1;

    RendererAPI<QGL_2_1>()->glUseProgram(0);
    
    GP_COLOR_PRINT(GP_COLOR::BRIGHT_GREEN, "Initialized: ");
    GP_PRINT("OpenGL_2_1_RenderDevice\n");
}

void OpenGL_2_1_RenderDevice::reset()
{
    GP_COLOR_PRINT(GP_COLOR::BRIGHT_GREEN, "Reset: ");
    printf("OpenGL_2_1_RenderDevice\n");
}

void OpenGL_2_1_RenderDevice::update(float layer)
{ 
    RendererAPI<QGL_2_1>()->glUseProgram(0);
    
    GP_TRACE("Entities count = ",  entities().count());
   
    if(layer == GL_LAYER_PICKABLE)
    {
        RendererAPI<QGL_2_1>()->glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
        RendererAPI<QGL_2_1>()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (auto Entity : entities().with<OpenGL_2_1_RenderKernel, commit_component>())
        {
            if(Entity.get<commit_component>().is_committed())
            {
            auto& render_kernel = Entity.get<OpenGL_2_1_RenderKernel>();
            bool  render_sucess = render_kernel.render_selection_mode();
            if(render_sucess)
            GP_TRACE("Entity : ", Entity.get<tag_component>().tag_name(), " Rendered in Select Mode");
            }
        }

        Event::Publisher::GetInstance()->frame_buffer_ogl_2_1()->update_current_frame_buffer(); 
        
        if(gp_std::is_debug_flag_set("GP_SELECTION_DEBUG"))
        {
           return;    
        }

        RendererAPI<QGL_2_1>()->glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
        RendererAPI<QGL_2_1>()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     
        return;
    }

    else if (layer == GL_LAYER_DISPLAY_ALL)
    {
        for (auto Entity : entities().with<OpenGL_2_1_RenderKernel, commit_component>())
        {
            if(Entity.get<commit_component>().is_committed())
            {
            auto& render_kernel = Entity.get<OpenGL_2_1_RenderKernel>();
            bool  render_sucess = render_kernel.render_display_mode();
            if(render_sucess)
            GP_TRACE("Entity : ", Entity.get<tag_component>().tag_name(), " Rendered in Display Mode"); 
            }  
        }
        return;
    }

    else
    {
        for (auto Entity : entities().with<OpenGL_2_1_RenderKernel, commit_component>())
        {
            auto& scene_state  = Event::Publisher::GetInstance()->get_scene_state();
            const float entity_layer_id = Entity.get<commit_component>().layer_id();
            if (entity_layer_id == layer || entity_layer_id == GL_LAYER_BACKGROUND)
            {
               if(entity_layer_id == GL_LAYER_FOREGROUND_2D || entity_layer_id == GL_LAYER_BACKGROUND_2D)
               {
                   scene_state.set_to_2d_mode();
               }

               if(Entity.get<commit_component>().is_committed())
               {
               auto& render_kernel = Entity.get<OpenGL_2_1_RenderKernel>();
               bool  render_sucess = render_kernel.render_display_mode();
               if(render_sucess)
               GP_TRACE("Entity : ", Entity.get<tag_component>().tag_name(), " Rendered in Display Mode");
               }
               
               if(entity_layer_id == GL_LAYER_FOREGROUND_2D || entity_layer_id == GL_LAYER_BACKGROUND_2D)
               {
                   scene_state.set_to_3d_mode();
               }
            }
        }
    }
}

} // namespace GridPro_GFX    
