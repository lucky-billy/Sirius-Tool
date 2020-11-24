QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    globalfun.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    globalfun.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resource.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#******************************************************************************************************************

TARGET = Sirius-Tool

DESTDIR = ./

CONFIG += debug_and_release

RC_ICONS = Sirius-Tool.ico

include("./Camera/camera.pri")

INCLUDEPATH += $$PWD/OpenCV_msvc2017_64/include

CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_calib3d420d
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_core420d
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_dnn420d
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_features2d420d
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_flann420d
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_gapi420d
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_highgui420d
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_imgcodecs420d
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_imgproc420d
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_ml420d
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_objdetect420d
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_photo420d
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_stitching420d
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_video420d
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_videoio420d
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/debug/ -lopencv_world420d
} else {
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_calib3d420
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_core420
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_dnn420
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_features2d420
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_flann420
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_gapi420
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_highgui420
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_imgcodecs420
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_imgproc420
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_ml420
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_objdetect420
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_photo420
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_stitching420
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_video420
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_videoio420
    LIBS += -L$$PWD/OpenCV_msvc2017_64/lib/release/ -lopencv_world420
}
