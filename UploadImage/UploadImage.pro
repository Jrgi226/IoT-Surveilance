TEMPLATE = app
TARGET = UploadImage

QT = core

HEADERS += \
    rcrremotestorageobj.h \
    certs.h
SOURCES += \
    main.cpp \
    rcrremotestorageobj.cpp \
    certs.c

#DEFINES += ENABLEWIDGETS
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

unix:!macx {
    INCLUDEPATH += /home/iot-1/azure-iot-sdk-c/c-utility/inc /home/iot-1/azure-iot-sdk-c/iothub_client/inc
    DEPENDPATH  += /home/iot-1/azure-iot-sdk-c/c-utility/inc /home/iot-1/azure-iot-sdk-c/iothub_client/inc
    LIBS        += -L/home/iot-1/azure-iot-sdk-c/cmake/iothub_client -liothub_client -liothub_client_amqp_transport \
                   -liothub_client_http_transport -liothub_client_mqtt_ws_transport \
                   -liothub_client_amqp_ws_transport -liothub_client_mqtt_transport -L/home/iot-1/azure-iot-sdk-c/cmake/c-utility -laziotsharedutil \
                   -lcrypto -lssl -lcurl

}

win32 {
    INCLUDEPATH += $$quote(C:/Users/ColePC/source/repos/azure-iot-sdk-c/c-utility/inc) \
                   $$quote(C:/Users/ColePC/source/repos/azure-iot-sdk-c/iothub_client/inc)
    DEPENDPATH  += $$quote(C:/Users/ColePC/source/repos/azure-iot-sdk-c/c-utility/inc) \
                   $$quote(C:/Users/ColePC/source/repos/azure-iot-sdk-c/iothub_client/inc)
    LIBS        += -L$$quote(C:/Users/ColePC/source/repos/azure-iot-sdk-c/cmake/iothub_client/Release) \
                   -L$$quote(C:/Users/ColePC/source/repos/azure-iot-sdk-c/cmake/c-utility/Release) \
                   -liothub_client_http_transport -liothub_client_mqtt_ws_transport \
                   -liothub_client_amqp_ws_transport -liothub_client_mqtt_transport \
                   -liothub_client_dll -laziotsharedutil \
                   -lws2_32 -lSecur32 -lwinhttp -lCrypt32 -lAdvapi32 -lNcrypt
}







