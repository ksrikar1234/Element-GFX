#ifndef _GP_IMGUI_WINDOW_HPP_
#define _GP_IMGUI_WINDOW_HPP_

//#ifdef GP_ENABLE_IMGUI
#include <string>
#include <memory>
#include <map>

class QOpenGLWidget;
class imgui_widget;

class imgui_window
{
public:
    imgui_window(QOpenGLWidget* parent_canvas_widget);
   ~imgui_window();

    bool add_widget(const char* name, std::shared_ptr<imgui_widget> widget);
    bool remove_widget(const char* name);

    void setup_render_pass();
    void render();
    
    void hide()   { m_visible = false;}
    void show()   { m_visible = true; }
    
    bool toggle()          { m_visible = !m_visible; return m_visible; }
    bool toggle(bool flag) { m_visible = flag; return m_visible; }

    bool is_visible() const { return m_visible; }

    bool want_to_capture_mouse_input() const;
    bool want_to_capture_keyboard_input() const;

    bool set_fonts(const char* font_path, float font_size);

private:
    QOpenGLWidget* m_parent_widget;
    std::map<std::string, std::shared_ptr<imgui_widget>> m_widgets;
    bool m_visible;
};

//#endif

#endif