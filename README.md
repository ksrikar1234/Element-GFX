This is GridPro GFX Module.
Add graphics_sources.pri file into your Application Pro file to integrate directly
or
You can Build a Shared or Static Library using graphics_shared_lib.pro file

GFX API Include Files are Placed in [(Graphics/gp_gfx_shared_lib/include)]
and Libs in [Graphics/gp_gfx_shared_lib/lib/(linux/mac/windows)]

GridPro GFX API consists of only two classes
1. GridPro_GFX::GeometryDescriptor
2. Viewer class which Inherits AbstractViewerWindow Class Public API Functions 

These two classes and their Methods Provide the all the necessary utilities to build Interactive 3D Graphics Scene Widget to 
Draw anything   

### TODO:
1. Add Center of Rotation for and ability pick accurately any point on an entity.
2. Stabilize Shader Driver. 
   Known issues :
   2.1 Using Background Quad has some issues. 
   2.2 is_view_in_motion for not drawing excess calls is working fine in opengl 2.1 but not with OpenGL3.3
      Selection rendering is having issues.It worked when removed but unable to replicate the sucess

3. Add Textures and Material Customisations for Phongs shading.
4. Add GLEW + GLFW API Viewer and Renderer API support.
5. Add a CMake Build file