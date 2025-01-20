#ifndef QT_OSG_VIEWER_HPP
#define QT_OSG_VIEWER_HPP

#include "base_viewer.h"

#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QWheelEvent>

#include <osg/Vec3d>
#include <osg/ref_ptr>
#include <osg/Geode>

#include <iostream>
#include <memory> 

namespace osgViewer {
  class Viewer;
  class GraphicsWindowEmbedded;
}

namespace GP_OSG {
  class OrthographicCamera;
}

namespace osgGA {
  class EventQueue;
}

namespace osg {
  class Geode;
  class Group;
  class Geometry;
}

/// @class  OSGViewer
/// @brief  OpenSceneGraph viewer class
/// @note   This class uses OpenSceneGraph to render
class LIB_API Viewer : public QOpenGLWidget, public AbstractViewerWindow
{
public:
  Viewer(QWidget* parent);

  virtual ~Viewer(){}

  void setScale(qreal X, qreal Y);

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
   
   void upload_commits() override final;

   void set_bounding_box(const std::vector<double> &in_bounding_box) override final;

protected:

   void paintGL() override;
   void resizeGL( int width, int height ) override;
   void initializeGL() override;
   void mouseMoveEvent(QMouseEvent* event) override;
   void mousePressEvent(QMouseEvent* event) override;
   void mouseReleaseEvent(QMouseEvent* event) override;
   void wheelEvent(QWheelEvent* event) override;
   bool event(QEvent* event) override;
  
private:
  osgGA::EventQueue* getEventQueue() const;

private:
  std::ref_ptr<osgViewer::Viewer> m_viewer;
  std::ref_ptr<osgViewer::GraphicsWindowEmbedded> m_graphics_window;
  std::shared_ptr<GP_OSG::OrthographicCamera> m_ortho_camera;

  // The root node of your scene graph
  osg::ref_ptr<osg::Geode> m_rootNode;

  // Stores the Head Node of Each Layer (Traversing the Head Node will give you all the other Geodes in that layer)
  std::map<float, osg::Geode*> m_layer_geodes; 

  qreal m_scaleX, m_scaleY;
  QPoint m_lastMousePosition; 
};

#endif // QT_OSG_VIEWER_HPP