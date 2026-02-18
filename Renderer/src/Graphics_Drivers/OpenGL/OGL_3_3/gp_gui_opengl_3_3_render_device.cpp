#include <iostream>

#include "gp_gui_opengl_3_3_render_device.h"
#include "gp_gui_opengl_3_3_render_kernel.h"

#include "gp_gui_opengl_3_3_shader.h"
#include "gp_gui_shader_library.h"
#include "gp_gui_shader_src.h"

#include "gp_gui_communications.h"

#include "gp_gui_opengl_3_3_framebuffer.h"

#include "graphics_api.hpp"

#ifndef USE_GLEW_OPENGL_API_ENTRY

#include <QOpenGLContext>
#include <QSurfaceFormat>

#endif

namespace GridPro_GFX {

using namespace OpenGL_3_3;

void OpenGL_3_3_RenderDevice::init()
{
    is_initialized = false;
    GP_PRINT("\nTrying to Initialise OpenGL_3_3_RenderDevice::init");
    
    #ifndef USE_GLEW_OPENGL_API_ENTRY

    QOpenGLContext* context = QOpenGLContext::currentContext();
    
    if (context) 
    {
        std::cout << "Current OpenGL context: " << context->format().version().first << "." << context->format().version().second << "\n";

        if(context->format().profile() != QSurfaceFormat::NoProfile)
            std::cout << "Profile: " << (context->format().profile() == QSurfaceFormat::CoreProfile ? "Core" : "Compatibility") << "\n";
        else 
            std::cout << "Profile: No Profile" << "\n";  
        
        int major_version = context->format().version().first;
        int minor_version = context->format().version().second;

        if(major_version < 3 || (major_version == 3 && minor_version < 3))
        {
            std::cerr << "OpenGL version 3.3 or higher is required\n";
            throw std::runtime_error("OpenGL version 3.3 or higher is required !! \n  Removing OpenGL_3_3_RenderDevice from the system\n");
        }
    }
    else
    {
        std::cout << "No current OpenGL context\n";
    }
    
    #endif // #ifndef USE_GLEW_OPENGL_API_ENTRY
    
    RendererAPI<QGL_3_3>()->initializeOpenGLFunctions();
    is_initialized = true;
    try
    {
        ShaderLibrary<OpenGL_3_3::Shader>::AddShader("BasicShader_" + std::to_string(m_render_context.id()), ShaderSrc::BasicVertexShaderSource, ShaderSrc::BasicFragmentShaderSource);
        ShaderLibrary<OpenGL_3_3::Shader>::AddShader("BasicPerVertexColorShader_" + std::to_string(m_render_context.id()), ShaderSrc::PerVertexColorVertexShaderSource, ShaderSrc::PerVertexColorFragmentShaderSource);
        ShaderLibrary<OpenGL_3_3::Shader>::AddShader("PhongsLightingShader_" + std::to_string(m_render_context.id()), ShaderSrc::PhongsLightingVertexShaderSource, ShaderSrc::PhongsLightingFragmentShaderSource);
        ShaderLibrary<OpenGL_3_3::Shader>::AddShader("SelectGeometryShader_" + std::to_string(m_render_context.id()), ShaderSrc::SelectGeometryVertexShaderSource, ShaderSrc::SelectGeometryFragmentShaderSource);
        ShaderLibrary<OpenGL_3_3::Shader>::AddShader("SelectPrimitiveShader_" + std::to_string(m_render_context.id()), ShaderSrc::SelectPrimitiveVertexShaderSource, ShaderSrc::SelectPrimitiveFragmentShaderSource);
        RendererAPI<QGL_3_3>()->glUseProgram(0);
    }

    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        throw e;
    }   
}

void OpenGL_3_3_RenderDevice::reset()
{
    printf("OpenGL_3_3_RenderDevice::reset\n");
    if(!is_initialized) return;
    ShaderLibrary<OpenGL_3_3::Shader>::ResetShaders(m_render_context.id());
    RendererAPI<QGL_3_3>()->glUseProgram(0);
}

void OpenGL_3_3_RenderDevice::update(float layer)
{ 
    RendererAPI<QGL_3_3>()->glUseProgram(0);
    // Instrumentation::Stopwatch watch("OpenGL_3_3_RenderDevice::update");
    GP_TRACE("Entities count = ",  entities().count());
   
    if(layer == GL_LAYER_PICKABLE)
    {
        RendererAPI<QGL_3_3>()->glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
        RendererAPI<QGL_3_3>()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto Entity : entities().with<OpenGL_3_3_RenderKernel, commit_component>())
        {
            if(Entity.get<commit_component>().is_committed())
            {
            auto& render_kernel = Entity.get<OpenGL_3_3_RenderKernel>();
            bool  render_sucess = render_kernel.render_selection_mode();
            if(render_sucess)
            GP_TRACE("Entity : ", Entity.get<tag_component>().tag_name(), " Rendered in Select Mode");
            }
        }

        Event::Publisher::GetInstance()->frame_buffer_ogl_3_3()->update_current_frame_buffer(); 

        RendererAPI<QGL_3_3>()->glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
        RendererAPI<QGL_3_3>()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);       
        
        return;
    }

    else if (layer == GL_LAYER_DISPLAY_ALL)
    {
        for (auto Entity : entities().with<OpenGL_3_3_RenderKernel, commit_component>())
        {
            if(Entity.get<commit_component>().is_committed())
            {            
            auto& render_kernel = Entity.get<OpenGL_3_3_RenderKernel>();
            bool  render_sucess = render_kernel.render_display_mode();
            Entity.get<commit_component>().set_rendered_in_display_mode(true);
            if(render_sucess)
            GP_TRACE("Entity : ", Entity.get<tag_component>().tag_name(), " Rendered in Display Mode");
            }
        }
        return;
    }

    else
    {
        for (auto Entity : entities().with<OpenGL_3_3_RenderKernel, commit_component>())
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
                auto& render_kernel = Entity.get<OpenGL_3_3_RenderKernel>();
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
    
    RendererAPI<QGL_3_3>()->glUseProgram(0);
}

} // namespace GridPro_GFX    
