#include "viewer.h"

#ifdef _GP_USE_OSG_VIEWER_H_

// Do not include this file if OSG Viewer is used

#else 

#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <map>

#include <QTimer>
#include <QString>

#include <glm/glm.hpp>

#include "graphics_api.hpp"

#include "gp_gui_communications.h"
#include "gp_gui_events.h"
#include "gp_gui_debug.h"

#ifndef OLD_WS
#include "communication/gp_publisher.h"
#endif

#include "gp_gui_scene.h"

#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#include "gp_imgui_mainwindow.hpp"
#include "gp_imgui_menu.hpp"
#include "gp_imgui_grid_viewer.hpp"

#include <QImage>
#include <QFileDialog>


QOpenGLFunctions* M_GL_API = nullptr;

/// @brief This Class is a Qt QOpenGLWidget that is used to render the 3D scene which inherits from AbstractViewerWindow
Viewer::Viewer(QWidget *parent) : QOpenGLWidget(parent), AbstractViewerWindow()
{
    GP_TRACE("Constructing Viewer Widget");
    QSurfaceFormat format;
    format.setSwapBehavior(QSurfaceFormat::TripleBuffer);
    format.setAlphaBufferSize(0);
    // format.setSamples(4); // For Anti Aliasing but has issue with glReadPixels 
    setFormat(format);

    QOpenGLWidget::setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_CurrentButton = Qt::NoButton;

    m_scroll_stop_detection_timer = new QTimer(this);
    m_scroll_stop_detection_timer->setSingleShot(true);
    
    connect(m_scroll_stop_detection_timer, &QTimer::timeout, this, [this]()
            {enable_selection_rendering = true; update(); });
    GP_TRACE("Successfully Initialised Viewer Widget");        
}

Viewer::~Viewer()
{
    accquire_render_context();
    m_scene.reset();
    delete m_scroll_stop_detection_timer;
    delete m_imgui_window;
    doneCurrent();
}

void Viewer::initializeGL()
{
    GP_TRACE("Initiatlising OpenGL Context");
    
    initialize_scene();
    
    M_GL_API = QOpenGLContext::currentContext()->functions(); 
    
    M_GL_API->glEnable(GL_DEPTH_TEST);
 
    m_imgui_window = new imgui_window(this);

    //m_imgui_window->add_widget("test",  make_imgui_widget<imgui_menu>());
    m_imgui_window->add_widget("grid_viewer", make_imgui_widget<imgui_grid_viewer>());
    
    m_imgui_window->set_fonts("*", 14.0f);
    //m_imgui_window->hide();
    GP_TRACE("Sucessfully Initialised OpenGL Context");
}

void Viewer::resizeGL(int w, int h)
{
    M_GL_API->glViewport(0, 0, w, h);
    resize_event(w, h);
}

void Viewer::paintGL()
{
    m_imgui_window->setup_render_pass();
    
    M_GL_API->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    M_GL_API->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render_update();

    m_imgui_window->render();
}

void Viewer::mousePressEvent(QMouseEvent *event)
{
    MouseButton button = MouseButton::None;
    QString button_string = "";
    switch (event->button())
    {
    case Qt::LeftButton:
        button = MouseButton::Left;
        button_string = "left";
        break;

    case Qt::RightButton:
        button = MouseButton::Right;
        button_string = "right";
        break;

    case Qt::MiddleButton:
        button = MouseButton::Middle;
        button_string = "middle";
        break;

    default:
        break;
    }

    if(m_imgui_window->want_to_capture_mouse_input()) return;

    mouse_press_event(event->x(), event->y(), button);

    gridpro_gui::Event::Subscription sub("mouse_press");

    if (sub.getPickEvent().getEntityKey() != "NULL_ENTITY")
    {
        m_CurrentButton = event->button();
        const std::string selected_entity_key = sub.getPickEvent().getEntityKey();

        std::map<QString, QString> entity_data;
        entity_data["entity_key"] = selected_entity_key.c_str();
        entity_data["sub_entity"] = std::to_string(sub.getPickEvent().getSubEntityID()).c_str();
        entity_data["button"]     = button_string;

        QString key_held = "none";

        if (m_ctrl_key)
        {
            key_held = "ctrl";
        }

        entity_data["key_held"] = key_held;
        
        #ifndef OLD_WS
        gridpro_comms::post_packet_async("mainwindow", "entity_selected", entity_data);
        #endif
        
        emit picked(entity_data);
    }
}

void Viewer::mouseReleaseEvent(QMouseEvent *event)
{
    MouseButton button = MouseButton::None;

    switch (event->button())
    {
    case Qt::LeftButton:
        button = MouseButton::Left;
        break;

    case Qt::RightButton:
        button = MouseButton::Right;
        break;

    case Qt::MiddleButton:
        button = MouseButton::Middle;
        break;

    default:
        break;
    }

    m_CurrentButton = Qt::NoButton;

    mouse_release_event(float(event->x()), float(event->y()), button);

    if(m_imgui_window->want_to_capture_mouse_input()) return;
}

void Viewer::mouseMoveEvent(QMouseEvent *event)
{
    if(m_imgui_window->want_to_capture_mouse_input()) return;

    mouse_move_event(event->x(), event->y());

    m_LastMousePosition = event->pos();
}

void Viewer::keyPressEvent(QKeyEvent *event)
{
    if(m_imgui_window->want_to_capture_keyboard_input()) return;

    switch (event->key())
    {
    case Qt::Key_Control:
    {
        m_ctrl_key = true;
        key_press_event(0);
        break;
    }
    case Qt::Key_Shift:
    {
        m_shift_key = true;
        key_press_event(1);
        break;
    }
    case Qt::Key_Alt:
    {
        m_alt_key = true;
        key_press_event(2);
        capture_screen_shot();
        break;
    }
    default:
        break;
    }
}

void Viewer::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Control:
        m_ctrl_key = false;
        break;
    case Qt::Key_Shift:
        m_shift_key = false;
        break;
    case Qt::Key_Alt:
        m_alt_key = false;
        break;
    case Qt::Key_Space:
        m_space_key = false;
        break;
    default:
        break;
    }
}

/// @brief This function is used to handle the mouse wheel event
/// @param event QWheelEvent
void Viewer::wheelEvent(QWheelEvent *event)
{
    if(m_imgui_window->want_to_capture_mouse_input()) return;

    wheel_event(event->angleDelta().y() / 120.0f);
    // If the user stops scrolling, we will enable the selection rendering in the last frame
    m_scroll_stop_detection_timer->start(150);
}

bool Viewer::add_imgui_widget(const char* in_widget_name, std::shared_ptr<imgui_widget> in_widget)
{
    return m_imgui_window->add_widget(in_widget_name, in_widget);
}

bool Viewer::remove_imgui_widget(const char* in_widget_name)
{
    return m_imgui_window->remove_widget(in_widget_name);
}

void Viewer::capture_screen_shot() 
{
    GP_PRINT("Capturing High-Resolution Screen Shot");

    // Desired high resolution
    int highResWidth  = this->width()   * 4;  
    int highResHeight = this->height()  * 8;

    makeCurrent();
    this->resize(highResWidth, highResHeight);
    this->update(); // Trigger rendering at the higher resolution

    // Wait for the widget to render completely
    QCoreApplication::processEvents();
    
    // Grab the FBO content as a QImage
    QImage image = grabFramebuffer();
    
    // Save the image
    QString savePath = QFileDialog::getSaveFileName(this, "Save Frame as JPEG", "", "TIFF Files (*.jpeg)");
    if (!savePath.isEmpty()) 
    {
        if (image.save(savePath, "JPEG")) {
            qDebug() << "High-resolution screenshot saved to" << savePath;
        } 
        else 
        {
            qWarning("Failed to save the high-resolution screenshot as JPEG}.");
        }
    }

    this->resize(highResWidth/4, highResHeight/8);
    this->update(); // Trigger rendering at the higher resolution

    // Wait for the widget to render completely
    QCoreApplication::processEvents();
}


#endif 