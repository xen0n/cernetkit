TEMPLATE = app

QT += qml quick widgets

SOURCES += main.cpp \
    networkhelper.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    networkhelperimplbase.h \
    networkhelper.h \
    univhelperimplbase.h \
    networkhelperdata.h \
    univhelperimpljiangnan.h


# platform implementations
win32 {
    SOURCES += \
        networkhelperimplwindows.cpp

    HEADERS += \
        networkhelperimplwindows.h
}

linux {
    SOURCES += \
        networkhelperimpllinux.cpp

    HEADERS += \
        networkhelperimpllinux.h
}


# University helper implementations
# TODO: real configurable university selection, hardcoded for now as there is
# only 1 university supported
DEFINES += UNIV_JIANGNAN

SOURCES += \
    univhelperimpljiangnan.cpp

HEADERS += \
    univhelperimpljiangnan.h


# link settings for g++ under Windows
win32-g++ {
    WindowsSdkDir = $$system(cmd /C windows-sdk-helper.bat)

    exists($$WindowsSdkDir) {
        LIBS += -lws2_32 -lole32 -loleaut32 -luuid -L"$$WindowsSdkDir\Lib" -lWbemUuid
    } else {
        error(Windows SDK is required to build this project.)
    }
}
