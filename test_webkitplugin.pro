QT    += gui core webkit network
TARGET = ftp_plug-in
TEMPLATE = app


HEADERS += \
    webpluginfactory.h \
    webview.h \
    ftpwindow.h

SOURCES += \
    webpluginfactory.cpp \
    webview.cpp \
    main.cpp \
    ftpwindow.cpp

FORMS +=

RESOURCES += \
    test_plugin.qrc
