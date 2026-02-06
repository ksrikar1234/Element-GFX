#ifndef _GP_OSG_CAMERA_HPP
#define _GP_OSG_CAMERA_HPP

#include <array>
#include <osg/ref_ptr>
#include <osg/Vec3d>
#include <osgViewer/Viewer>
#include <osgViewer/GraphicsWindow>
#include <osg/Camera>


namespace GP_OSG
{
   /// @class OrthographicCamera
   /// @brief Inhouse implementation of an orthographic camera manipulation class
   
   class BoundingBox
   {
    public:
    BoundingBox() : m_min(0.0, 0.0, 0.0), m_max(0.0, 0.0, 0.0) {}
    BoundingBox(const osg::Vec3d& min, const osg::Vec3d& max) : m_min(min), m_max(max) {}

    BoundingBox(const BoundingBox& other) : m_min(other.m_min), m_max(other.m_max) {}

    BoundingBox& operator=(const BoundingBox& other) 
    {
        m_min = other.m_min;
        m_max = other.m_max;
        return *this;
    }

    BoundingBox& operator=(BoundingBox&& other) noexcept
    {
        m_min = std::move(other.m_min);
        m_max = std::move(other.m_max);
        return *this;
    }

    void set_min(const osg::Vec3d& min) { m_min = min; }
    void set_max(const osg::Vec3d& max) { m_max = max; }
    
    osg::Vec3d get_min() const { return m_min; }
    osg::Vec3d get_max() const { return m_max; }
    
    osg::Vec3d get_center() const { return (m_min + m_max) / 2.0; }

    osg::Vec3d get_size() const { return m_max - m_min; }   

    osg::Vec3d get_extent() const { return get_size() / 2.0; }

    void set_center(const osg::Vec3d& center)
    {
        osg::Vec3d extent = get_extent();
        m_min = center - extent;
        m_max = center + extent;
    }

    void set_size(const osg::Vec3d& size)
    {
        osg::Vec3d center = get_center();
        osg::Vec3d extent = size / 2.0;
        m_min = center - extent;
        m_max = center + extent;
    }

    std::array<osg::Vec3, 8>  get_nodes()
    {
        std::array<osg::Vec3, 8> nodes;
        nodes[0] = osg::Vec3(m_min.x(), m_min.y(), m_min.z());
        nodes[1] = osg::Vec3(m_max.x(), m_min.y(), m_min.z());
        nodes[2] = osg::Vec3(m_max.x(), m_max.y(), m_min.z());
        nodes[3] = osg::Vec3(m_min.x(), m_max.y(), m_min.z());
        nodes[4] = osg::Vec3(m_min.x(), m_min.y(), m_max.z());
        nodes[5] = osg::Vec3(m_max.x(), m_min.y(), m_max.z());
        nodes[6] = osg::Vec3(m_max.x(), m_max.y(), m_max.z());
        nodes[7] = osg::Vec3(m_min.x(), m_max.y(), m_max.z());
        return nodes;
    }

    osg::Vec3 get_camera_position(const osg::Vec3d& camera_orientation, const osg::Vec3d& camera_up_direction)
    {
        osg::Vec3d center = get_center();
        osg::Vec3d extent = get_extent();
        osg::Vec3d ortho_direction = camera_orientation ^ camera_up_direction;
        ortho_direction.normalize();
        osg::Vec3d camera_position = center - ortho_direction * extent.x() * 2.0;
        return camera_position;
    }

    private:
    osg::Vec3d m_min;
    osg::Vec3d m_max;
   };



   class OrthographicCamera
   {
    public:
    OrthographicCamera(osgViewer::GraphicsWindowEmbedded* in_viewer, int x, int y, int width, int height) : 
          m_orthoWidth(100.0) // Initial orthographic width
        , m_cameraPosition(osg::Vec3d(0.0, 0.0, 0.0)) // Initial camera position
        , m_cameraOrientation(0.0, 0.0, -1.0)  // Looking along -Z axis
        , m_cameraUpDirection(0.0, 1.0, 0.0)   // Y axis is up
        , m_orthoDirection(1.0, 0.0, 0.0)      // X axis is right
        , m_currCenterOfRotation(0.0, 0.0, 0.0) // Center of the scene
    {
      camera = new osg::Camera;
      camera->setViewport(x, y, width, height);
      camera->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
      camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
      camera->setGraphicsContext(in_viewer);
    }

    operator osg::Camera*() { return camera.get(); }
    
    void setClearColor(const float& r, const float& g, const float& b, const float& a)
    {
        camera->setClearColor(osg::Vec4(r, g, b, a));
    }

    void setCenterOfRotation(const osg::Vec3d& centerOfRotation)
    {
        m_currCenterOfRotation = centerOfRotation;
    }

    void setViewport(int x, int y, int width, int height)
    {
       camera->setViewport(x, y, width, height);
    }

    void set_bounding_box(const std::vector<double> &in_bounding_box)
    {
        BoundingBox bounding_box(osg::Vec3d(in_bounding_box[0], in_bounding_box[1], in_bounding_box[2]),
                                 osg::Vec3d(in_bounding_box[3], in_bounding_box[4], in_bounding_box[5]));
        m_boundingBox = bounding_box;

        // Set the center of rotation to the center of the bounding box
        m_currCenterOfRotation = m_boundingBox.get_center();

        // Set the camera position
        m_cameraPosition = m_boundingBox.get_camera_position(m_cameraOrientation, m_cameraUpDirection);

        // Set the orthographic width
        m_orthoWidth = m_boundingBox.get_extent().x() * 2.0;

        updateProjectionMatrix();
        updateViewMatrix();
    } 

    void translateCamera(float xoffset, float yoffset)
    {
        // Get viewport dimensions
        osg::Viewport* viewport = getCamera()->getViewport();
        double viewportWidth = viewport->width();
        double viewportHeight = viewport->height();

        // Calculate deltas
        double del_x = xoffset * (m_right - m_left) / viewportWidth;
        double del_y = yoffset * (m_top - m_bottom) / viewportHeight;

        // Compute ortho_direction (right vector)
        m_orthoDirection = m_cameraOrientation ^ m_cameraUpDirection;
        m_orthoDirection.normalize();

        // Adjust camera position and center of rotation
        m_cameraPosition -= m_orthoDirection * del_x;
        m_cameraPosition += m_cameraUpDirection * del_y;

        m_currCenterOfRotation -= m_orthoDirection * del_x;
        m_currCenterOfRotation += m_cameraUpDirection * del_y;

        updateViewMatrix();
    }

    void rotateCamera(float deltaX, float deltaY)
    {
        // Get viewport dimensions
        osg::Viewport* viewport = getCamera()->getViewport();
        double width = viewport->width();
        double height = viewport->height();

        // Calculate rotation angles
        double theta = (-deltaX) * osg::PI / width;   // Yaw
        double phi = deltaY * osg::PI / height;       // Pitch

        // Normalize orientation vectors
        m_cameraOrientation.normalize();
        m_cameraUpDirection.normalize();
        m_orthoDirection = m_cameraOrientation ^ m_cameraUpDirection; // Cross product
        m_orthoDirection.normalize();

        // Create rotation matrices
        osg::Matrixd rotationMatrixYaw;
        rotationMatrixYaw.makeRotate(theta, m_cameraUpDirection);
        osg::Matrixd rotationMatrixPitch;
        rotationMatrixPitch.makeRotate(phi, m_orthoDirection);

        osg::Matrixd rotationMatrix = rotationMatrixYaw * rotationMatrixPitch;

        // Rotate orientation and up vectors
        m_cameraOrientation = rotationMatrix.preMult(m_cameraOrientation);
        m_cameraOrientation.normalize();

        m_cameraUpDirection = rotationMatrix.preMult(m_cameraUpDirection);
        m_cameraUpDirection.normalize();

        // Re-orthogonalize the up vector to ensure it's perpendicular to the orientation vector
        m_cameraUpDirection = m_cameraUpDirection - m_cameraOrientation * (m_cameraUpDirection * m_cameraOrientation);
        m_cameraUpDirection.normalize();

        // Update ortho direction
        m_orthoDirection = m_cameraOrientation ^ m_cameraUpDirection;
        m_orthoDirection.normalize();

        // Rotate camera position around the center of rotation
        m_cameraPosition -= m_currCenterOfRotation;
        m_cameraPosition = rotationMatrix.preMult(m_cameraPosition);
        m_cameraPosition += m_currCenterOfRotation;
        
        //m_cameraPosition = m_boundingBox.get_camera_position(m_cameraOrientation, m_cameraUpDirection);
        // Update the view matrix
        camera->setViewMatrixAsLookAt(
            m_cameraPosition,
            m_cameraPosition + m_cameraOrientation,
            m_cameraUpDirection);

        
        updateViewMatrix();


    }

    void zoomCamera(int delta)
    {
        // Compute the zoom ratio
        double zoomSensitivity = 0.15; // 15% zoom per unit delta
        double ratio = pow(1.0 + zoomSensitivity, -delta);

        // Ensure ratio remains within reasonable bounds
        ratio = std::max(0.01, std::min(100.0, ratio));

        // Get the center of zoom
        osg::Vec3d center_of_zoom = m_cameraPosition + m_cameraOrientation;

        // Calculate the vector from the center of zoom to the camera position
        osg::Vec3d offset = m_cameraPosition - center_of_zoom;

        // Compute the distance along the camera orientation
        double distance_along_orientation = offset * m_cameraOrientation;

        // Remove the component along the camera orientation to get the perpendicular offset
        osg::Vec3d perpendicular_offset = offset - m_cameraOrientation * distance_along_orientation;

        // Scale the perpendicular offset
        perpendicular_offset *= ratio;

        // Compute the new camera position
        m_cameraPosition = center_of_zoom + perpendicular_offset + m_cameraOrientation * distance_along_orientation;

        // Adjust m_orthoWidth
        m_orthoWidth *= ratio;

        // Ensure m_orthoWidth remains within reasonable bounds
        m_orthoWidth = std::max(0.1, std::min(100000.0, m_orthoWidth));

        // Update the projection and view matrices
        updateProjectionMatrix();
        updateViewMatrix();
    }


    void zoomCamera2(int delta)
    {
        double scaleFactor = 1.0;
        if (delta > 0)
        scaleFactor = 0.9; // Zoom in
        else
        scaleFactor = 1.1; // Zoom out
         
        m_orthoWidth *= scaleFactor;

        // Prevent the view from becoming too small or too large
        m_orthoWidth = std::max(0.1, std::min(100000.0, m_orthoWidth));

        updateProjectionMatrix();
    }

    void updateViewMatrix()
    {
        osg::Vec3d eye = m_cameraPosition;
        osg::Vec3d center = m_cameraPosition + m_cameraOrientation;
        osg::Vec3d up = m_cameraUpDirection;

        camera->setViewMatrixAsLookAt(eye, center, up);
    }

    void updateProjectionMatrix()
    {
        double aspectRatio = static_cast<double>(this->width()) / static_cast<double>(this->height());
        double halfWidth = m_orthoWidth / 2.0;
        double halfHeight = halfWidth / aspectRatio;

        halfHeight *= 2.0f;
        halfWidth *= 2.0f;
        
        m_left = -halfWidth;
        m_right = halfWidth;
        m_bottom = -halfHeight;
        m_top = halfHeight;

        double zNear = -1000000.0;
        double zFar = 1000000.0;

        camera->setProjectionMatrixAsOrtho(m_left, m_right, m_bottom, m_top, zNear, zFar);
    }

    int width() const
    {
        return camera->getViewport()->width();
    }

    int height() const
    {
        return camera->getViewport()->height();
    }

    osg::Camera* getCamera() const
    {
        return camera.get();
    }

    BoundingBox get_bounding_box() const
    {
        return m_boundingBox;
    }

    osg::Vec3d get_camera_position() const
    {
        return m_cameraPosition;
    }

    osg::Vec3d get_camera_orientation() const
    {
        return m_cameraOrientation;
    }

    osg::Vec3d get_camera_up_direction() const
    {
        return m_cameraUpDirection;
    }

    osg::Vec3d get_ortho_direction() const
    {
        return m_orthoDirection;
    }

    osg::Vec3d get_curr_center_of_rotation() const
    {
        return m_currCenterOfRotation;
    }

    osg::Vec4 get_head_light_position() const
    {
        return osg::Vec4(m_cameraPosition, 1.0);
    }

    private:
    osg::ref_ptr<osg::Camera> camera;
    
    BoundingBox m_boundingBox;

    osg::Vec3d m_cameraOrientation;    // Direction the camera is looking at
    osg::Vec3d m_cameraUpDirection;    // Up direction of the camera
    osg::Vec3d m_orthoDirection;       // Right direction (orthogonal to orientation and up)
    osg::Vec3d m_currCenterOfRotation; // Center of rotation  
  
    double m_orthoWidth;
    osg::Vec3d m_cameraPosition;

    double m_left;
    double m_right;
    double m_bottom;
    double m_top;
    

    osgViewer::Viewer* _mViewer;
  
   };
}


#endif // _GP_OSG_CAMERA_HPP