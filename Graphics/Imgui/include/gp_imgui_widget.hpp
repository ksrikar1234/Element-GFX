#ifndef _GP_IMGUI_WIDGET_HPP_
#define _GP_IMGUI_WIDGET_HPP_

#include <memory>
#include <type_traits>

class imgui_widget
{
public:
    imgui_widget() = default;
    virtual ~imgui_widget() = default;

    virtual void render() = 0;

    void show() { m_visible = true; }
    void hide() { m_visible = false; }
    bool toggle()          { m_visible = !m_visible; return m_visible; }
    bool toggle(bool flag) { m_visible = flag;       return m_visible; }
    bool is_visible() const { return m_visible; }

protected:
    bool m_visible = true;
};


template <typename T, typename... Args>
std::shared_ptr<imgui_widget> make_imgui_widget(Args&&... args)
{
    static_assert(std::is_base_of<imgui_widget, T>::value, "T must inherit from imgui_widget");
    return std::make_shared<T>(std::forward<Args>(args)...);
}

#endif
