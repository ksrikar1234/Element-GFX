#ifndef _GP_QT_OLD_WS_VIEWER_WIDGET_H_
#define _GP_QT_OLD_WS_VIEWER_WIDGET_H_

//#define _FOR_OLD_WS_GP_GUI_GLWIDGET_

// ** For Old Workspace **
#ifdef  _FOR_OLD_WS_GP_GUI_GLWIDGET_

#include <QGLWidget>
using Gp_gui_glwidget_type = QGLWidget;

#else

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Compatibility>
using Gp_gui_glwidget_type = QOpenGLWidget;

#endif

#include <QMouseEvent>

#include "glm/glm.hpp"
#include "base_viewer.h"


class QTimer;

namespace gridpro_gui {
/// @brief LegacyViewer class is a Symbiotic class that upgrades the Existing Gp_gui_GLWidget with new Drivers
/// This class is a bridge between the Older Gp_gui_GLWidget and the new Viewer class
class LIB_API LegacyViewer :  public AbstractViewerWindow
{

typedef void (*ViewerUpdateGLCallback)();

public:
    ///************** Public API *****************

    /// @brief  Constructor
    /// @param parent
    LegacyViewer(Gp_gui_glwidget_type* parent =  nullptr);
    void setParent(Gp_gui_glwidget_type* parent);

    /// @brief  Destructor
    ~LegacyViewer() override final;
        
    void initializeGL();
    void paintGL(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &model, const glm::vec3 &camera_position);

    void mousePressEvent(QMouseEvent* event);

    bool mouseMoveEvent(QMouseEvent* event) ;
    void mouseReleaseEvent(QMouseEvent* event);

    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

    void wheelEvent(QWheelEvent *event);
    
    /// @brief  This function is accquire the render context
    /// @return bool
    bool accquire_render_context() override
    {
        m_parent->makeCurrent();
        return true;
    }

    /// @brief  This function is used to update the display
    void update_display() override;
 
    float device_pixel_ratio() const override
    {
        return (float)m_parent->devicePixelRatio();
    } 

    void set_update_gl_callback(ViewerUpdateGLCallback callback)
    {
        m_update_gl_callback = callback;
    }   
    
    template<typename T>
    void set_update_gl_callback(T* instance, void (T::*callback)())
    {
        m_update_gl_callback = [instance, callback]()
        {
            if(callback)
                (instance->*callback)();
        };
    }
    
private:
    Gp_gui_glwidget_type* m_parent;
    QPoint m_LastMousePosition;
    Qt::MouseButton m_CurrentButton;
    QTimer* m_scroll_stop_detection_timer;
    std::function<void()> m_update_gl_callback;
    glm::mat4 m_projection, m_view;

};
} // namespace gridpro_gui
#endif // ENGINE_DEMO_H
