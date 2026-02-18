
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

#include <iostream>
#include <vector>

#include <iostream>
#include <filesystem>
#include <QSurfaceFormat>

#include <QCoreApplication>
#include <QDir>
#include <iostream>

#include "PythonModule/PythonModules.hpp"

#include "../../renderer.h"

class PythonConsole : public QWidget
{
    Q_OBJECT
public:
    PythonConsole(Viewer *v, QWidget *parent);

public Q_SLOTS:
    void runScript(); // Declare here, define outside

private:
    Viewer *viewer;
    QPlainTextEdit *editor;
    QPlainTextEdit *output;
};


void initialize_gridpro_python();

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

    initialize_gridpro_python();
    
    // Start the interpreter here (stays alive for the whole app duration)
    py::scoped_interpreter guard{};

    QMainWindow window;

    window.setWindowTitle("GridPro Scripting Viewer");
    window.showMaximized();

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


inline PythonConsole::PythonConsole(Viewer *v, QWidget *parent = nullptr) : QWidget(parent), viewer(v)
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


// Define the function OUTSIDE the class block
// This often helps MOC when working with single-file projects
inline void PythonConsole::runScript()
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
        // Instead of casting the object directly, cast the address to a long long
        // locals["mainwindow_ptr"] = reinterpret_cast<uintptr_t>(viewer->parent());
        // locals["mainwindow"] = py::cast(viewer->parent(), py::return_value_policy::reference);
        // globals["gfx"] = gfx;

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

void initialize_gridpro_python()
{
    QString binDir = QApplication::applicationDirPath();
    QString pyHomeStr = binDir + "/runtime";
    
    // We must use static variables or ensure these pointers stay valid 
    // until Py_Initialize is called.
    static std::wstring pyHome = pyHomeStr.toStdWString();
    static std::wstring pyExec = (pyHomeStr + "/bin/python3").toStdWString();

    // 1. Critical: Clear the environment
    unsetenv("PYTHONPATH");
    unsetenv("PYTHONHOME");

    // 2. These MUST be called before py::scoped_interpreter guard{}
    Py_SetPythonHome(pyHome.c_str());
    Py_SetProgramName(pyExec.c_str());
    
    // 3. Optional but helpful for debugging
    std::wcout << L"Setting Home to: " << pyHome << std::endl;
}

#include "qt_app.moc"  