#-------------------------------------------------
#
# Project created by QtCreator 2019-10-13T18:02:12
#
#-------------------------------------------------
# qtbase5-dev
QT += core gui
# qtmultimedia5-dev,
QT += multimediawidgets network
#unix{
    QT += webengine webenginewidgets
#}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtipcam
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# require libmpv-dev:amd64    0.32.0-1ubuntu1
QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig debug
PKGCONFIG += mpv

#CONFIG += c++11

SOURCES += \
        src/main.cpp \
        src/mainwindow.cpp \
    src/dialogcfg.cpp

HEADERS += \
    lib/qthelper.hpp \
        src/mainwindow.h \
    src/camtype.h \
    src/dialogcfg.h

FORMS += \
        src/mainwindow.ui \
    src/dialogcfg.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
else: unix:!android: target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qtipcam.qrc

DISTFILES += \
    README.md

unix {
    desktop.path  = /usr/share/applications/
    desktop.files += \
        qtipcam.desktop

    images.path = /usr/share/pixmaps/
}
win32 {
    desktop.path = $${DESTDIR}
    images.path =  $${DESTDIR}
}

images.files += \
        images/qtipcam.png

INSTALLS += \
    images \
    desktop
