#include "../include/Viewers/viewer.h"

#ifdef _GP_USE_OSG_VIEWER_H_

#include "osg_viewer.h"

#include <osgViewer/GraphicsWindow>
#include <osgViewer/Viewer>
#include "gp_osg_camera.hpp"
#include "gp_osg_geometry.hpp"

#include <osg/Geode>
#include <osg/Group>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/Material>
#include <osgGA/EventQueue>

#include "gp_gui_geometry_descriptor.h"
#include "gp_gui_scene.h"
#include "gp_gui_debug.h"

#include <iostream>
#include <glm/glm.hpp>

using namespace gridpro_gui;

Viewer::Viewer(QWidget* parent)
        : QOpenGLWidget(parent)
        , m_graphics_window(new osgViewer::GraphicsWindowEmbedded(this->x(), this->y(),
                                                                 this->width(), this->height()))
        , m_viewer(new osgViewer::Viewer)
        , m_scaleX(1.0)
        , m_scaleY(1.0)


{    

    m_ortho_camera = std::make_shared<GP_OSG::OrthographicCamera>(m_graphics_window.get(), 0, 0, this->width(), this->height());
    m_ortho_camera->setCenterOfRotation(osg::Vec3d(0.5f, 1.0f, 0.0f));
    m_ortho_camera->setClearColor(0.117647, 0.117647, 0.117647, 0.117647);

    // Set up orthographic projection
    m_ortho_camera->updateProjectionMatrix();

    // Set up the view matrix
    m_ortho_camera->updateViewMatrix();

    m_viewer->setCamera((*m_ortho_camera));
    
    // Create the root node
    m_rootNode =  new osg::Geode();
    m_viewer->setSceneData(m_rootNode);
    
    m_center_descriptor = std::make_shared<GeometryDescriptor>();
    create_and_display_axes();
    upload_commits();
    
    m_ortho_camera->getCamera()->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::ON);
    m_viewer->setLightingMode(osg::View::HEADLIGHT);


    // osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource();
    // osg::ref_ptr<osg::Light> light = new osg::Light();
    // light->setLightNum(0);
    // light->setDiffuse(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));
    // light->setSpecular(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));

    // // Set initial position of the light
    // light->setPosition(osg::Vec4(0.0f, 0.0f, 100.0f, 1.0f)); // Positional light

    // lightSource->setLight(light.get());
    // lightSource->setReferenceFrame(osg::LightSource::ABSOLUTE_RF);

    // m_rootNode->addChild(lightSource.get());

    // Remove the camera manipulator
    m_viewer->setCameraManipulator(nullptr);

    this->setMouseTracking(true);
    m_viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    m_viewer->realize();
}

void Viewer::setScale(qreal X, qreal Y)
{
    m_scaleX = X;
    m_scaleY = Y;
    this->resizeGL(this->width(), this->height());
}


void Viewer::initializeGL()
{
    m_scene = std::make_shared<Scene_Manager>();
    m_center_descriptor = std::make_shared<GeometryDescriptor>();

    is_scene_created_successfully = m_scene->has_render_device();

    if (is_scene_created_successfully == false)
    {
        GP_ERROR("Init GL Scene not created successfully\n");
    }

    m_scene->switch_driver(GL_DRIVER_OPENGL_2_1);

    create_and_display_axes();
    
    osg::Geode* geode = dynamic_cast<osg::Geode*>(m_viewer->getSceneData());
    osg::StateSet* stateSet = geode->getOrCreateStateSet();
    osg::Material* material = new osg::Material;
    material->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
    stateSet->setAttributeAndModes(material, osg::StateAttribute::ON);
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
}

void Viewer::resizeGL(int width, int height)
{
    this->getEventQueue()->windowResize(this->x(), this->y(), width, height);
    m_graphics_window->resized(this->x(), this->y(), width, height);
    osg::Camera* camera = m_viewer->getCamera();
    camera->setViewport(0, 0, width, height);
    m_ortho_camera->updateProjectionMatrix();
}

void Viewer::paintGL()
{
    upload_commits();
    m_viewer->frame();
}

void Viewer::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)
    {
        m_lastMousePosition = event->pos();
    }
}

void Viewer::mouseMoveEvent(QMouseEvent* event)
{
    QPoint delta = event->pos() - m_lastMousePosition;
    m_lastMousePosition = event->pos();

    if (event->buttons() & Qt::LeftButton)
    {
        m_ortho_camera->translateCamera(delta.x(), delta.y());
        m_viewer->getLight()->setPosition(m_ortho_camera->get_head_light_position());
        update();
    }
    else if (event->buttons() & Qt::MiddleButton)
    {
        m_ortho_camera->rotateCamera(delta.x(), delta.y());
        m_viewer->getLight()->setPosition(m_ortho_camera->get_head_light_position());
        update();
    }
}

void Viewer::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)
    {
        m_lastMousePosition = event->pos();
    }
}

void Viewer::wheelEvent(QWheelEvent* event)
{
    m_ortho_camera->zoomCamera(event->angleDelta().y() / 120);
    update();
}

bool Viewer::event(QEvent* event)
{
    bool handled = QOpenGLWidget::event(event);
    this->update();
    return handled;
}

osgGA::EventQueue* Viewer::getEventQueue() const
{
    osgGA::EventQueue* eventQueue = m_graphics_window->getEventQueue();
    return eventQueue;
}


void Viewer::upload_commits()
{
    for(size_t i = m_geometry_commit_stack.size(); i > 0; --i)
    {
        auto &commit = m_geometry_commit_stack[i - 1];
        GP_OSG::Geometry geometry(commit.geometry);
        m_rootNode->addChild(geometry.get_geometry());
    }
    m_geometry_commit_stack.clear();
}

void Viewer::set_bounding_box(const std::vector<double> &in_bounding_box)
{
    m_ortho_camera->set_bounding_box(in_bounding_box);
}


#endif
