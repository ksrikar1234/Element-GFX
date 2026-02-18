#include "viewer.h"

#ifdef _GP_USE_OSG_VIEWER_H_
// Do not include this file if OSG Viewer is used
#else 
#include <map>

#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#include <QImage>
#include <QFileDialog>
#include <QTimer>
#include <QString>

#include <QPainter>

#include <glm/glm.hpp>
#include "gp_gui_scene.h"
#include "gp_gui_camera.h"

#include "gp_gui_communications.h"
#include "gp_gui_events.h"
#include "gp_gui_debug.h"

#ifndef OLD_WS
#include "communication/gp_publisher.h"
#endif

#include "graphics_api.hpp"


static QOpenGLFunctions* M_GL_API = nullptr;

/// @brief This Class is a Qt QOpenGLWidget that is used to render the 3D scene which inherits from AbstractViewerWindow
Viewer::Viewer(QWidget* parent) : QOpenGLWidget(parent), AbstractViewerWindow()
{
    GP_TRACE("Constructing Viewer Widget");

    QSurfaceFormat format;
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setDepthBufferSize(24);
    format.setAlphaBufferSize(4);
    // format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setStencilBufferSize(8); 
    // format.setSamples(4); // For Anti Aliasing but has issue with glReadPixels 

    setFormat(format);

    QOpenGLWidget::setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_CurrentButton = Qt::NoButton;

    m_scroll_stop_detection_timer = new QTimer(this);
    m_scroll_stop_detection_timer->setSingleShot(true);
    
    connect(m_scroll_stop_detection_timer, &QTimer::timeout, this, [this]() {enable_selection_rendering = true; update(); });

    GP_TRACE("Successfully Initialised Viewer Widget"); 

    setContextMenuPolicy(Qt::CustomContextMenu);    

    connect(this, &QOpenGLWidget::customContextMenuRequested,
            this, []{ std::cout<<"CC GL\n"; });         
}

Viewer::~Viewer()
{
    accquire_render_context();
    m_scene.reset();
    delete m_scroll_stop_detection_timer;
    //delete m_imgui_window;
    doneCurrent();
}

void Viewer::initializeGL()
{
    GP_TRACE("Initiatlising OpenGL Context");
    
    initialize_scene();
    
    M_GL_API = QOpenGLContext::currentContext()->functions(); 
    
    M_GL_API->glEnable(GL_DEPTH_TEST);
 
    //m_imgui_window = new imgui_window(this);

    //m_imgui_window->add_widget("test",  make_imgui_widget<imgui_menu>());
    //m_imgui_window->add_widget("grid_viewer", make_imgui_widget<imgui_grid_viewer>());
    
    //m_imgui_window->set_fonts("*", 14.0f);
    //m_imgui_window->hide();
    GP_TRACE("Sucessfully Initialised OpenGL Context");

    QSurfaceFormat fmt = this->format();
    qDebug() << "--- OpenGL Surface Diagnostic ---";
    qDebug() << "Vendor:  " << (const char*)M_GL_API->glGetString(GL_VENDOR);
    qDebug() << "Renderer:" << (const char*)M_GL_API->glGetString(GL_RENDERER);
    qDebug() << "RGBA Bits [" << fmt.redBufferSize() << "," 
             << fmt.greenBufferSize() << "," 
             << fmt.blueBufferSize() << "," 
             << fmt.alphaBufferSize() << "]";
    qDebug() << "Depth Bit Size:" << fmt.depthBufferSize();
    qDebug() << "Profile: " << (fmt.profile() == QSurfaceFormat::CoreProfile ? "Core" : "Compatibility");
    qDebug() << "---------------------------------";
}

void Viewer::resizeGL(int w, int h)
{
    M_GL_API->glViewport(0, 0, w, h);
    resize_event(w, h);
}

void Viewer::paintGL()
{
    //m_imgui_window->setup_render_pass();
    M_GL_API->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    M_GL_API->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render_update();

    //m_imgui_window->render();
}

void Viewer::mousePressEvent(QMouseEvent *event)
{
    MouseButton button = MouseButton::None;
    switch (event->button())
    {
    case Qt::LeftButton :
        button = (m_shift_key) ? MouseButton::Middle : MouseButton::Left;
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

   // if(m_imgui_window->want_to_capture_mouse_input()) return;

    mouse_press_event(event->x(), event->y(), button);

    GridPro_GFX::Event::Subscription sub("mouse_press");

    if (sub.getPickEvent().getEntityKey() != "NULL_ENTITY")
    {
        m_CurrentButton = event->button();
        const std::string selected_entity_key = sub.getPickEvent().getEntityKey();

        int key_held = -1;

        if (m_ctrl_key)
        {
            key_held = Qt::Key_Control;
        }

       uint32_t sub_entity = sub.getPickEvent().getSubEntityID();
        
        Q_EMIT gl_entity_picked(selected_entity_key, sub_entity, m_CurrentButton, key_held, QPoint(event->x(), event->y()));
       // emit picked(entity_data);
    }
}

void Viewer::mouseReleaseEvent(QMouseEvent* event)
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

    // if(m_imgui_window->want_to_capture_mouse_input()) return;
}

void Viewer::mouseMoveEvent(QMouseEvent *event)
{
    //if(m_imgui_window->want_to_capture_mouse_input()) return;

    mouse_move_event(event->x(), event->y());

    m_LastMousePosition = event->pos();
}

void Viewer::keyPressEvent(QKeyEvent *event)
{
    //if(m_imgui_window->want_to_capture_keyboard_input()) return;

    switch (event->key())
    {
    case Qt::Key_Control:
    {
        m_ctrl_key = true;
        break;
    }
    case Qt::Key_Shift:
    {
        m_shift_key = true;
        break;
    }
    case Qt::Key_Alt:
    {
        m_alt_key = true;
        key_press_event(1);
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
    // if(m_imgui_window->want_to_capture_mouse_input()) return;

    wheel_event(event->angleDelta().y() / 120.0f);
    // If the user stops scrolling, we will enable the selection rendering in the last frame
    m_scroll_stop_detection_timer->start(150);
}

bool Viewer::add_imgui_widget(const char* in_widget_name, std::shared_ptr<imgui_widget> in_widget)
{
    return true; 
    //m_imgui_window->add_widget(in_widget_name, in_widget);
}

bool Viewer::remove_imgui_widget(const char* in_widget_name)
{
    return true ;
    // m_imgui_window->remove_widget(in_widget_name);
}

void Viewer::capture_screen_shot(uint32_t res_scale_x, uint32_t res_scale_y) 
{
    GP_PRINT("Capturing High-Resolution Screen Shot");
    if(res_scale_x == 0 || res_scale_y == 0)
    {
        throw std::runtime_error("Invalid Resolution Scale Factor. Cannot Be '0' \n");
    }

    int highResWidth  = this->width()  * res_scale_x;  
    int highResHeight = this->height() * res_scale_y;

    makeCurrent();
    this->resize(highResWidth, highResHeight);
    this->update(); 

    // Wait for the widget to render completely
    QCoreApplication::processEvents();
    
    // Grab the FBO content as a QImage [Need to use this for Read Pixels in Render Kernel]
    QImage image = grabFramebuffer();
    
    QString savePath = QFileDialog::getSaveFileName(this, "Save Frame as JPEG", "", "JPEG Files (*.jpeg)");
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

    this->resize(highResWidth/res_scale_x, highResHeight/res_scale_y);
    this->update(); 

    QCoreApplication::processEvents();
}

void Viewer::render_2d_text(double normalised_x, double normalised_y,
                            std::string text_to_be_rendered,
                            size_t txt_size, uint32_t color_code)
{
    QColor color((color_code >> 16) & 0xFF,
                 (color_code >> 8) & 0xFF,
                  color_code & 0xFF);

    text_overlays.push_back
    (
      TextOverlay
      {
        normalised_x,
        normalised_y,
        QString::fromStdString(text_to_be_rendered),
        static_cast<int>(txt_size),
        color
      }
    );
}

void Viewer::paintEvent(QPaintEvent* e) 
{
    QOpenGLWidget::paintEvent(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setPen(Qt::red);
    
    for (const auto& overlay : text_overlays) 
    {
        int x = static_cast<int>(overlay.nx * width());
        int y = static_cast<int>(overlay.ny * height());
        painter.setFont(QFont("Arial", overlay.size));
        painter.drawText(x, y, overlay.text);
    }

    painter.end();
}


#endif 