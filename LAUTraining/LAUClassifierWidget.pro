#-------------------------------------------------
#
# Project created by QtCreator 2018-02-27T11:26:16
#
#-------------------------------------------------

QT       += core gui widgets opengl

TARGET = LAUClassifierWidget
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

SOURCES += \
        main.cpp \
        laumemoryobject.cpp \
        lauvideoglwidget.cpp \
        lauclassifierwidget.cpp \
        laumachinelearningobject.cpp

HEADERS += \
        laumemoryobject.h \
        lauvideoglwidget.h \
        lauclassifierwidget.h \
        laumachinelearningobject.h

RESOURCES += \
        lauclassifierwidget.qrc

unix:!macx {
    QMAKE_CXXFLAGS += -msse2 -msse3 -mssse3 -msse4.1
    INCLUDEPATH    += /usr/local/include # /usr/include/eigen3 /usr/local/include/opencv2
    DEPENDPATH     += /usr/local/include # /usr/include/eigen3 /usr/local/include/opencv2
    LIBS           += -ltiff # -lopencv_core -lopencv_ml
    CONFIG         += link_pkgconfig
    PKGCONFIG      += opencv
}

win32 {
    INCLUDEPATH    += $$quote(C:/usr/opencv/build/include) $$quote(C:/usr/include)
    DEPENDPATH     += $$quote(C:/usr/opencv/build/include) $$quote(C:/usr/include)
    LIBS           += -L$$quote(C:/usr/opencv/build/x64/vc14/lib) -lopencv_world310d -L$$quote(C:/usr/lib) -llibtiff_i -llibtiff -lopengl32
}
