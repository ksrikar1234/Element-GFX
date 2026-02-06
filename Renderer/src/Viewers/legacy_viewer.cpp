
#include "legacy_viewer.h"

#include <map>

#include <QTimer>
#include <QString>

#include <glm/glm.hpp>

#include "graphics_api.hpp"

#include "gp_gui_scene.h"
#include "gp_gui_camera.h"

#include "gp_gui_communications.h"
#include "gp_gui_events.h"
#include "gp_gui_debug.h"

#include "gp_gui_typedefs.h"

using namespace GridPro_GFX;

LegacyViewer::LegacyViewer(Gp_gui_glwidget_type* parent) : AbstractViewerWindow(), m_parent(parent)
{
    m_CurrentButton = Qt::NoButton;
    if(m_parent != nullptr)
    {
       setParent(m_parent);
    }
}

void LegacyViewer::setParent(Gp_gui_glwidget_type* parent)
{
    m_parent = parent;
 
    // m_scroll_stop_detection_timer = new QTimer(parent);
    // m_scroll_stop_detection_timer->setSingleShot(true);

    // parent->connect(m_scroll_stop_detection_timer, &QTimer::timeout, this, [this]()
    //         {enable_selection_rendering = true; update_display(); });
}

LegacyViewer::~LegacyViewer()
{
    
}

void LegacyViewer::initializeGL()
{
    is_scene_created_successfully = true;

    m_scene = std::make_shared<Scene_Manager>();
    m_camera = std::make_shared<OrthographicCamera>();

    is_scene_created_successfully = m_scene->has_render_device();

    if (is_scene_created_successfully == false)
    {
        GP_ERROR("Init GL Scene not created successfully\n");
    }

    m_MouseSensitivity = 0.02f;

    set_lighting_state(true);

    m_scene->switch_driver(GL_DRIVER_OPENGL_2_1);

    m_scene->get_scene_state().set_framebuffer_enum(GridPro_GFX::SceneState::FrameBufferEnum::OpenGL_2_1_FRONT_BUFFER);
}


void LegacyViewer::paintGL(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &model, const glm::vec3 &camera_position)
{
    m_scene->set_mvp(projection, view, model);

    set_light_position(camera_position.x, camera_position.y, camera_position.z);
    
    upload_commits();

    m_scene->commit_all();

    for(auto& hidden_geometry : m_hidden_geometries)
    {
        m_scene->uncommit_entity(hidden_geometry);
    }

    m_scene->update((GL_LAYER_PICKABLE));
    
    for(auto& active_layer_id : m_current_displayed_layers)
    {
        m_scene->update(active_layer_id);
    }
}

void LegacyViewer::mousePressEvent(QMouseEvent *event)
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

    mouse_press_event(event->x(), event->y(), button);
}

void LegacyViewer::mouseReleaseEvent(QMouseEvent *event)
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
}

bool LegacyViewer::mouseMoveEvent(QMouseEvent *event)
{
    m_LastMousePosition = event->pos();
    return mouse_move_event(event->x(), event->y());
}

void LegacyViewer::keyPressEvent(QKeyEvent *event)
{

}

void LegacyViewer::keyReleaseEvent(QKeyEvent *event)
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
void LegacyViewer::wheelEvent(QWheelEvent *event)
{
    wheel_event(event->angleDelta().y() / 120.0f);
    // If the user stops scrolling, we will enable the selection rendering in the last frame
    m_scroll_stop_detection_timer->start(150);
}

void LegacyViewer::update_display() 
{  
    m_update_gl_callback();
}