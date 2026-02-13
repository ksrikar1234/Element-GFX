# GridPro Python Embedding Integration
# Supports: macOS (Static/Universal), Linux (ManyLinux), Windows (Embedded)

# 1. Common pybind11 Configuration
INCLUDEPATH += $$PWD
    message("Configuring Python 3.11")
# ---------------------------------------------------------
# macOS Configuration
# ---------------------------------------------------------
macx {
    INCLUDEPATH += $$PWD/mac/compile_time/include
}

# ---------------------------------------------------------
# Linux Configuration (RHEL 7.9 / ManyLinux)
# ---------------------------------------------------------
unix:!macx {
    INCLUDEPATH += $$PWD/linux/compile_time/include
}

# ---------------------------------------------------------
# Windows Configuration
# ---------------------------------------------------------
win32 {
    INCLUDEPATH += $$PWD/windows/include
}
