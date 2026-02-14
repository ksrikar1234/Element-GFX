# GridPro Python Embedding Integration
# Supports: macOS (Static/Universal), Linux (ManyLinux), Windows (Embedded)

# 1. Common pybind11 Configuration
INCLUDEPATH += $$PWD
CONFIG += c++17

# ---------------------------------------------------------
# macOS Configuration
# ---------------------------------------------------------
macx {
    message("Configuring Python 3.11 for macOS (Universal Static)")

    INCLUDEPATH += $$PWD/mac/compile_time/include
    QMAKE_LFLAGS += -rdynamic

    QMAKE_LFLAGS += -Wl,-export_dynamic
    QMAKE_LFLAGS += -Wl,-undefined,dynamic_lookup

    PRE_TARGETDEPS += $$PWD/mac/compile_time/libs/libpython3.11.a
    QMAKE_LFLAGS += -Wl,-force_load, $$PWD/mac/compile_time/libs/libpython3.11.a

    QMAKE_LFLAGS += -Wl, -all_load

    LIBS += $$PWD/mac/compile_time/libs/libpython3.11.a

    # System dependencies for static Python on Mac
    LIBS += -ldl -lpthread -lz -lm
    LIBS += -framework CoreFoundation -framework Foundation
}

# ---------------------------------------------------------
# Linux Configuration (RHEL 7.9 / ManyLinux)
# ---------------------------------------------------------
unix:!macx {
    message("Configuring Python 3.11 for Linux")

    INCLUDEPATH += $$PWD/linux/compile_time/include
    
    # Link to the static library
    LIBS += -L$$PWD/linux/compile_time/libs -lpython3.11
    
    # Linux specific system libs (util and crypt are often required)
    LIBS += -lpthread -ldl -lutil -lm -lcrypt
}

# ---------------------------------------------------------
# Windows Configuration
# ---------------------------------------------------------
win32 {
    message("Configuring Python 3.11 for Windows")

    INCLUDEPATH += $$PWD/windows/include
    
    # On Windows, we link against the import library (.lib)
    # The actual engine is in python311.dll which must be in the same folder as the exe
    LIBS += -L$$PWD/windows/libs -lpython311
}
