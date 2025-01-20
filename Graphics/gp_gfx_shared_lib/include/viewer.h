//#define _GP_USE_OSG_VIEWER_H_

#ifdef _GP_USE_OSG_VIEWER_H_

#include "osg_viewer.h"

#else 

#ifndef _GP_QT_VIEWER_WIDGET_H_
#define _GP_QT_VIEWER_WIDGET_H_


#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Compatibility>
#include <QMouseEvent>

#include "base_viewer.h"

class QTimer;

class imgui_window;
class imgui_widget;

/// @brief Viewer class is a Qt QOpenGLWidget that is used to render the 3D scene which also inherits from AbstractViewerWindow
/// @note Public API
/// @note commit_geometry() - This function is used to commit the geometry to the scene
/// @note set_bounding_box() - This function is used to set the bounding box of the scene
/// @note is_blending_enabled() - This function is used to check if blending is enabled
/// @note set_blend_state() - This function is used to enable or disable blending
/// @note is_lighting_enabled() - This function is used to check if lighting is enabled
/// @note set_lighting_state() - This function is used to enable or disable lighting

class LIB_API Viewer : public QOpenGLWidget, public AbstractViewerWindow
{
    Q_OBJECT
public:
    ///************** Public API *****************

    /// @brief  Constructor
    /// @param parent
    Viewer(QWidget *parent = nullptr);

    /// @brief  Destructor
    virtual ~Viewer() override;
    
    /// @brief  This function is accquire the render context
    /// @return bool
    bool accquire_render_context() override
    {
        makeCurrent();
        return true;
    }

    /// @brief  This function is used to update the display
    void update_display() override
    {
        update();
    }
 
    float device_pixel_ratio() const override
    {
        return (float)this->devicePixelRatio();
    }

    bool add_imgui_widget(const char* in_widget_name, std::shared_ptr<imgui_widget> in_widget);

    bool remove_imgui_widget(const char* in_widget_name);

    imgui_window* im_window() 
    {
        return m_imgui_window;
    }

    signals:
    void picked(std::map<QString, QString> data);

    /// ************** Public API End *****************
protected:
    void initializeGL() override;

    void resizeGL(int w, int h) override;

    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;
        
private:
    QPoint m_LastMousePosition;
    Qt::MouseButton m_CurrentButton;
    QTimer* m_scroll_stop_detection_timer;

    imgui_window* m_imgui_window;
};
#endif // _GP_USE_OSG_VIEWER_H_
#endif // ENGINE_DEMO_H
