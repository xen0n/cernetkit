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
    networkhelper.h


win32-g++ {
    WindowsSdkDir = $$system(cmd /C windows-sdk-helper.bat)

    exists($$WindowsSdkDir) {
        LIBS += -lws2_32 -lole32 -loleaut32 -luuid -L"$$WindowsSdkDir\Lib" -lWbemUuid
    } else {
        error(Windows SDK is required to build this project.)
    }
}
