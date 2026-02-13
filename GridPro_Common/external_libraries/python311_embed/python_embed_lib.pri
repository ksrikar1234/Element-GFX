# ---------------------------------------------------------
# macOS Configuration
# ---------------------------------------------------------
macx {
    message("Configuring Python 3.11 for macOS (Universal Static)")

    LIBS += $$PWD/mac/compile_time/libs/libpython3.11.a
    LIBS += -ldl -lpthread -lz -lm
    LIBS += -framework CoreFoundation -framework Foundation
}

# ---------------------------------------------------------
# Linux Configuration (RHEL 7.9 / ManyLinux)
# ---------------------------------------------------------
unix:!macx {
    message("Configuring Python 3.11 for Linux")
    LIBS += -L$$PWD/linux/compile_time/libs -lpython3.11
    LIBS += -lpthread -ldl -lutil -lm -lcrypt
}

# ---------------------------------------------------------
# Windows Configuration
# ---------------------------------------------------------
win32 {
    message("Configuring Python 3.11 for Windows")
    # The actual engine is in python311.dll which must be in the same folder as the exe
    LIBS += -L$$PWD/windows/libs -lpython311
}
