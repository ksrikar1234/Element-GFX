#pragma once

#include <iostream>
#include <vector>
#include <filesystem>
#include "Pybind11.hpp"

#include "../../renderer.h"

namespace py = pybind11;
namespace fs = std::filesystem;

PYBIND11_EMBEDDED_MODULE(GridPro_GFX, m)
{
    // 1. GeometryDescriptor Binding
    py::class_<GridPro_GFX::GeometryDescriptor, std::shared_ptr<GridPro_GFX::GeometryDescriptor>>(m, "GeometryDescriptor")
        .def(py::init<>())
        .def("set_current_primitive_set", &GridPro_GFX::GeometryDescriptor::set_current_primitive_set)
        // Wrapper for positions (NumPy -> std::vector<float>)
        .def("copy_pos_array", [](GridPro_GFX::GeometryDescriptor &self, py::array_t<float> array)
        {
            auto buf = array.request();
            float* ptr = static_cast<float*>(buf.ptr);
            std::vector<float> data(ptr, ptr + buf.size);
            self.copy_pos_array(data); 
        })
        // Wrapper for indices (NumPy -> std::vector<uint32_t>)
        .def("copy_index_array", [](GridPro_GFX::GeometryDescriptor &self, py::array_t<uint32_t> array)
        {
            auto buf = array.request();
            uint32_t* ptr = static_cast<uint32_t*>(buf.ptr);
            std::vector<uint32_t> data(ptr, ptr + buf.size);
            self.copy_index_array(data); 
        })

        .def("set_fill_color", &GridPro_GFX::GeometryDescriptor::set_fill_color)
        .def("set_wireframe_color", &GridPro_GFX::GeometryDescriptor::set_wireframe_color)
        .def("set_line_width", &GridPro_GFX::GeometryDescriptor::set_line_width);

    // 2. Viewer Binding with Overload Resolution
    py::class_<Viewer>(m, "Viewer")
        .def("commit_geometry", py::overload_cast<const std::string &, const std::shared_ptr<GridPro_GFX::GeometryDescriptor> &>(&Viewer::commit_geometry))
        // Mapping the simple commit
        .def("commit_geometry", [](Viewer &self, const std::string &name, const std::shared_ptr<GridPro_GFX::GeometryDescriptor> &input_geometry)
             { self.commit_geometry(name, input_geometry); })

        // Mapping the layered commit
        .def("commit_geometry_in_layer", [](Viewer &self, const std::string &name, const float &layer_id, const std::shared_ptr<GridPro_GFX::GeometryDescriptor> &input_geometry)
             { self.commit_geometry(name, layer_id, input_geometry); })

        // Getter
        .def("get_geometry", &Viewer::get_geometry)
        .def("hide_geometry", &Viewer::hide_geometry)
        .def("show_geometry", &Viewer::show_geometry)
        .def("update", &Viewer::update_display);

    // 3. Constants
    m.attr("GL_TRIANGLES")      = (int)GL_TRIANGLES;
    m.attr("GL_POINTS")         = (int)GL_POINTS;
    m.attr("GL_LINES")          = (int)GL_LINES;
    m.attr("GL_LINE_STRIP")     = (int)GL_LINE_STRIP;
    m.attr("GL_LINE_LOOP")      = (int)GL_LINE_LOOP;
    m.attr("GL_TRIANGLES")      = (int)GL_TRIANGLES;
    m.attr("GL_TRIANGLE_STRIP") = (int)GL_TRIANGLE_STRIP;
    m.attr("GL_TRIANGLE_FAN")   = (int)GL_TRIANGLE_FAN;
    m.attr("GL_QUADS")          = (int)GL_QUADS;
    m.attr("GL_QUAD_STRIP")     = (int)GL_QUAD_STRIP;  
}