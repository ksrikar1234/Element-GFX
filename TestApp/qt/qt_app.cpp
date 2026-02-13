
#include <QSurfaceFormat>
#include <QApplication>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QPalette>
#include <QTime>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include "../../renderer.h"

#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <iostream>
#include <vector>

#include <iostream>
#include <filesystem>
#include <QSurfaceFormat>

#include <Python.h>
#include <QCoreApplication>
#include <QDir>
#include <iostream>

void initialize_gridpro_python() {
    // 1. Get the absolute path to your runtime
    QString bundlePath = QCoreApplication::applicationDirPath();
    QString pyHome = bundlePath + "/runtime";
    
    // 2. Convert to Wide String for the C-API
    std::wstring wHome = pyHome.toStdWString();
    
    // 3. CRITICAL: Clear any environment variables that might be poisoning the path
    unsetenv("PYTHONPATH");
    unsetenv("PYTHONHOME");

    // 4. Set the Home explicitly
    Py_SetPythonHome(wHome.c_str());

    // 5. Hard-set the path to include only your bundle folders
    // This bypasses the logic that was looking in /Users/.../GridPro
    std::wstring wPath = wHome + L"/lib/python3.11:" + 
                         wHome + L"/lib/python3.11/lib-dynload:" +
                         wHome + L"/lib/python3.11/site-packages";
    Py_SetPath(wPath.c_str());

    // 6. Initialize (or use your pybind11 guard after this)
    Py_Initialize();
    
    if (!Py_IsInitialized()) {
        std::cerr << "Failed to initialize embedded Python!" << std::endl;
    }
}


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
            self.copy_pos_array(data); })

        // Wrapper for indices (NumPy -> std::vector<uint32_t>)
        .def("copy_index_array", [](GridPro_GFX::GeometryDescriptor &self, py::array_t<uint32_t> array)
             {
            auto buf = array.request();
            uint32_t* ptr = static_cast<uint32_t*>(buf.ptr);
            std::vector<uint32_t> data(ptr, ptr + buf.size);
            self.copy_index_array(data); })

        .def("set_fill_color", &GridPro_GFX::GeometryDescriptor::set_fill_color)
        .def("set_wireframe_color", &GridPro_GFX::GeometryDescriptor::set_wireframe_color)
        .def("set_line_width", &GridPro_GFX::GeometryDescriptor::set_line_width);

    // 2. Viewer Binding with Overload Resolution
    py::class_<Viewer>(m, "Viewer")
        .def("commit_geometry", py::overload_cast<const std::string &, const std::shared_ptr<GridPro_GFX::GeometryDescriptor> &>(&Viewer::commit_geometry))
        // Mapping the simple commit
        .def("commit_geometry",[](Viewer& self, const std::string& name,  const std::shared_ptr<GridPro_GFX::GeometryDescriptor> & input_geometry)
        {
            self.commit_geometry(name, input_geometry);     
        })

        // Mapping the layered commit
        .def("commit_geometry_in_layer",[](Viewer& self, const std::string& name, const float& layer_id,  const std::shared_ptr<GridPro_GFX::GeometryDescriptor> & input_geometry)
        {
            self.commit_geometry(name, layer_id, input_geometry);     
        })

        // Getter
        .def("get_geometry", &Viewer::get_geometry)
        .def("hide_geometry", &Viewer::hide_geometry)
        .def("show_geometry", &Viewer::show_geometry)
        .def("update", &Viewer::update_display);

    // 3. Constants
    m.attr("GL_TRIANGLES") = (int)GL_TRIANGLES;
}

class PythonConsole : public QWidget
{
    Q_OBJECT
public:
    PythonConsole(Viewer *v, QWidget *parent = nullptr) : QWidget(parent), viewer(v)
    {
        // Use 'new' for the layout to be safe
        auto layout = new QVBoxLayout(this);

        editor = new QPlainTextEdit(this);
        editor->setPlainText("import GridPro_GFX as gfx\nimport numpy as np\nprint('Hello from Python!')");

        output = new QPlainTextEdit(this);
        output->setReadOnly(true);
        output->setStyleSheet("background-color: #1e1e1e; color: #d4d4d4;");

        auto runBtn = new QPushButton("Run Script", this);

        layout->addWidget(editor, 3);
        layout->addWidget(runBtn, 0);
        layout->addWidget(output, 2);

        // Explicitly use the pointer-to-member syntax for connect
        connect(runBtn, &QPushButton::clicked, this, &PythonConsole::runScript);
    }

    // Try using public Q_SLOTS to ensure MOC visibility
public Q_SLOTS:
    void runScript(); // Declare here, define outside

private:
    Viewer *viewer;
    QPlainTextEdit *editor;
    QPlainTextEdit *output;
};

// Define the function OUTSIDE the class block
// This often helps MOC when working with single-file projects
void PythonConsole::runScript()
{

    // 1. Capture std::cout (C++)
    std::stringstream cpp_buffer;
    std::streambuf *old_cout = std::cout.rdbuf(cpp_buffer.rdbuf());

    try
    {
        // 1. Setup Module & Variables
        py::module_ gfx = py::module_::import("GridPro_GFX");
        auto globals = py::globals();
        auto locals = py::dict();
        locals["viewer"] = py::cast(viewer, py::return_value_policy::reference);
        globals["gfx"] = gfx;

        // 2. Setup Redirection BEFORE Execution
        py::module_ sys = py::module_::import("sys");
        py::module_ io = py::module_::import("io");
        py::object stringIO = io.attr("StringIO")();
        sys.attr("stdout") = stringIO;
        sys.attr("stderr") = stringIO; // Also catch errors if you want

        // 3. Execution logic
        py::exec(editor->toPlainText().toStdString(), globals, locals);

        // 3. Combine both outputs
        // 4. AFTER execution, fetch the results from the buffer
        std::string stdOut = py::cast<std::string>(stringIO.attr("getvalue")());

        std::string cppOut = cpp_buffer.str();

        if (!stdOut.empty())
        {
            QString timeStr = QTime::currentTime().toString("hh:mm:ss");
            output->appendPlainText(QString("[%1] %2").arg(timeStr, QString::fromStdString(stdOut)));
        }

        if (!cppOut.empty())
            output->appendPlainText(QString("[C++] " + QString::fromStdString(cppOut)));

        // 5. Cleanup: Reset stdout so future internal Python logic doesn't get trapped
        // (Optional but recommended for stability)
        sys.attr("stdout") = sys.attr("__stdout__");
    }
    catch (py::error_already_set &e)
    {
        output->appendHtml(QString("<br><b style='color:red'>Python Error:</b><br>%1")
                               .arg(QString::fromStdString(e.what()).toHtmlEscaped()));
    }

    // 4. RESTORE std::cout (Very important, or app logs will vanish!)
    std::cout.rdbuf(old_cout);
}

struct coord
{
    double x, y, z;
};

void load_geometry_file(std::string filename, Viewer *viewer);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage %s <filename>\n", argv[0]);
        return -1;
    }

    QApplication app(argc, argv);

    // 2. Get the internal path
    // This points to: ws_qt.app/Contents/MacOS/runtime
    QString binDir = QApplication::applicationDirPath();
    QString pyHome = binDir + "/runtime";

    // 3. YOUR ORIGINAL APPROACH: setenv
    // We MUST clear the 'GridPro/lib' path from your logs to prevent SIGABRT
    unsetenv("PYTHONPATH"); 
    
    // Set the HOME. Python 3.11 will read this during Py_Initialize.
    setenv("PYTHONHOME", pyHome.toUtf8().constData(), 1);

    // Start the interpreter here (stays alive for the whole app duration)
    py::scoped_interpreter guard{};

    QMainWindow window;

    window.setWindowTitle("GridPro Scripting Viewer");
    window.resize(1200, 800);

    // --- 2. SETUP VIEWER ---
    auto viewer = new Viewer(&window);
    viewer->setAutoFillBackground(false);
    viewer->setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);

    window.setCentralWidget(viewer);

    // --- 3. ADD SCRIPTING CONSOLE ---
    auto dock = new QDockWidget("Python Console", &window);
    auto console = new PythonConsole(viewer, dock);
    dock->setWidget(console);
    window.addDockWidget(Qt::RightDockWidgetArea, dock);

    // Load initial files from CLI
    for (int i = 1; i < argc; ++i)
    {
        load_geometry_file(std::string(argv[i]), viewer);
    }

    window.show();
    return app.exec();
}

void load_geometry_file(std::string filename, Viewer *viewer)
{
    std::vector<coord> points;
    std::vector<uint32_t> faces;

    std::ifstream file(filename.c_str());
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }

    size_t num_points = 0;
    file >> num_points;
    points.resize(num_points);

    coord min, max;

    for (size_t i = 0; i < num_points; ++i)
    {
        file >> points[i].x >> points[i].y >> points[i].z;
    }

    if (filename.size() > 5 && filename.substr(filename.size() - 5) == ".tria")
    {
        std::cout << "Loading GridPro .tria file\n";

        size_t num_trias = 0;
        file >> num_trias;
        faces.resize(num_trias * 3);

        std::cout << "Num Trias = " << num_trias << "\n";

        for (size_t i = 0; i < num_trias; ++i)
        {
            file >> faces[(3 * i) + 0];
            file >> faces[(3 * i) + 1];
            file >> faces[(3 * i) + 2];
            uint32_t ignore;
            file >> ignore;
        }

        for (size_t i = 0; i < num_trias; ++i)
        {
            faces[i * 3 + 0] -= 1;
            faces[i * 3 + 1] -= 1;
            faces[i * 3 + 2] -= 1;
        }

        std::cout << "Reading Done\n";
    }

    else if (filename.size() >= 5 && filename.substr(filename.size() - 5) == ".quad")
    {
        std::cout << "Loading GridPro .quad file\n";

        size_t num_quads = 0;
        file >> num_quads;
        faces.resize(num_quads * 4);

        for (size_t i = 0; i < num_quads; ++i)
        {
            file >> faces[(4 * i) + 0];
            file >> faces[(4 * i) + 1];
            file >> faces[(4 * i) + 2];
            file >> faces[(4 * i) + 3];
            uint32_t ignore;
            file >> ignore;
        }

        for (size_t i = 0; i < num_quads; ++i)
        {
            faces[i * 4 + 0] -= 1;
            faces[i * 4 + 1] -= 1;
            faces[i * 4 + 2] -= 1;
            faces[i * 4 + 3] -= 1;
        }
        std::cout << "Reading Quad File Done\n";
    }

    std::vector<float> gl_point(points.size() * 3);

    for (uint32_t i = 0; i < points.size(); ++i)
    {
        gl_point[3 * i] = points[i].x;
        gl_point[3 * i + 1] = points[i].y;
        gl_point[3 * i + 2] = points[i].z;

        min.x = std::min(min.x, points[i].x);
        min.y = std::min(min.y, points[i].y);
        min.z = std::min(min.z, points[i].z);

        max.x = std::max(min.x, points[i].x);
        max.y = std::max(max.y, points[i].y);
        max.z = std::max(max.z, points[i].z);
    }

    auto face_descriptor = std::make_shared<GridPro_GFX::GeometryDescriptor>();
    const std::string face_name = "cad_mesh";

    face_descriptor->set_current_primitive_set(face_name, GL_TRIANGLES);
    face_descriptor->move_pos_array(std::move(gl_point));
    face_descriptor->move_index_array(std::move(faces));

    // Visual options (as per your example)
    face_descriptor->set_fill_color(60, 120, 255, 255);
    face_descriptor->set_pick_scheme(GL_PICK_GEOMETRY);
    face_descriptor->set_wireframe_mode(GL_WIREFRAME_OVERLAY);
    face_descriptor->set_wireframe_color(255, 255, 255, 255);
    face_descriptor->set_line_width(3);

    // Commit to GL layer
    viewer->commit_geometry("MESHFACE", face_descriptor);
}

#include "qt_app.moc"