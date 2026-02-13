include(external_libraries/python311_embed/python_embed.pri)

SOURCES += \
    $$PWD/src/communication/gp_packet.cpp \
    $$PWD/src/communication/gp_slot.cpp \
    $$PWD/src/communication/gp_publisher.cpp \
    $$PWD/src/communication/gp_reciever.cpp 

HEADERS += \
    $$PWD/include/communication/gp_packet.h \
    $$PWD/include/communication/gp_slot.h \
    $$PWD/include/communication/gp_publisher.h \
    $$PWD/include/communication/gp_reciever.h 

INCLUDEPATH += $$PWD/include/

