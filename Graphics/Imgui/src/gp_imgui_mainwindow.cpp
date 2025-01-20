#include "gp_imgui_mainwindow.hpp"
#include "gp_imgui_widget.hpp"

#include "QtImGui.h"
#include "imgui.h"

#include <QOpenGLWidget>

// #ifdef _WIN32
// #include <windows.h>
// #endif

#include <filesystem>

bool is_ready_to_render = false;

imgui_window::imgui_window(QOpenGLWidget *parent_canvas_widget)
    : m_parent_widget(parent_canvas_widget), m_visible(true)
{
    QtImGui::initialize(parent_canvas_widget);
}

imgui_window::~imgui_window()
{

}

bool imgui_window::add_widget(const char* name, std::shared_ptr<imgui_widget> widget)
{
    if(m_widgets.find(std::string(name)) == m_widgets.end())
    {
        m_widgets[name] = widget;
        return true;
    }
    else
    {
        printf("Widget with name %s already exists\n", name);
        return false;
    }
}

bool imgui_window::remove_widget(const char* name)
{
    if(m_widgets.find(std::string(name)) != m_widgets.end())
    {
        m_widgets.erase(name);
        return true;
    }
    else
    {
        printf("Widget with name %s does not exist\n", name);
        return false;
    }
}

void imgui_window::setup_render_pass()
{
    if (m_visible)
    {
        QtImGui::newFrame();

        for(auto& widget_info : m_widgets)
        {
            widget_info.second->render();
        }

        is_ready_to_render = true;
    }
}

void imgui_window::render()
{
    if (m_visible)
    {
        if (is_ready_to_render)
        {
            ImGui::Render();
            QtImGui::render();
            is_ready_to_render = false;
        }
        else
        {
            printf("ImGui Pre-render Pass Not Setup\n");
        }
    }
}

bool imgui_window::want_to_capture_mouse_input() const
{
    if(!is_visible()) return false;
    
    bool is_mouse_input_required = ImGui::GetIO().WantCaptureMouse;
    
    if(is_mouse_input_required)
    {
        m_parent_widget->update();
    }

    return is_mouse_input_required;
}

bool imgui_window::want_to_capture_keyboard_input() const
{
    if(!is_visible()) return false;
    
    bool is_keyboard_input_required = ImGui::GetIO().WantCaptureKeyboard;
    
    if(is_keyboard_input_required)
    {
        m_parent_widget->update();
    }

    return is_keyboard_input_required;
}

bool imgui_window::set_fonts(const char* font_path, float font_size)
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
     
    const char* font_path_cstr = font_path;

    if(font_path_cstr == nullptr)
    {
        printf("Font Path is NULL\n");
    }
    else
    {
        #ifndef _WIN32
        font_path_cstr = "/usr/share/fonts/truetype/freefont/FreeSans.ttf";
        #elif __APPLE__
        font_path_cstr = "/Library/Fonts/Arial.ttf";
        #else
        font_path_cstr = "C:/Windows/Fonts/Arial.ttf";
        #endif
    }

    io.Fonts->AddFontFromFileTTF(font_path_cstr, font_size);
    io.Fonts->Build();
    return true;
}