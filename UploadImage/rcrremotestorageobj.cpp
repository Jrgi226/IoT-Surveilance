#include "rcrremotestorageobj.h"

RCRAzureIOTStorageObject::RCRAzureIOTStorageObject(QObject *parent) : RCRRemoteStorageObject(parent), iotHubClientHandle(NULL)
{
    qDebug() << "Object is being created and AzureIoT connection is being attempted";

    if (platform_init() != 0) {
        errorString = QString("Failed to initialize the platform.");
    } else {
        if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectStringPrimaryKey, HTTP_Protocol)) == NULL) {
            errorString = QString("ERROR: iotHubClientHandle is NULL!");
        } else {
            if (IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_TRUSTED_CERT, certificates) != IOTHUB_CLIENT_OK) {
                errorString = QString("failure in IoTHubClient_LL_SetOption (TrustedCerts)");
            } else {
                HTTP_PROXY_OPTIONS http_proxy_options = { NULL, 0, NULL, NULL };
                http_proxy_options.host_address = proxyHost;
                http_proxy_options.port = proxyPort;

                if (proxyHost != NULL && IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_HTTP_PROXY, &http_proxy_options) != IOTHUB_CLIENT_OK) {
                    errorString = QString("failure to set proxy");
                } else {
                    errorString = QString();
                }
            }
        }
    }
}

RCRAzureIOTStorageObject::~RCRAzureIOTStorageObject()
{
    if (isValid()){
        IoTHubClient_LL_Destroy(iotHubClientHandle);
        platform_deinit();
    }
}

bool RCRAzureIOTStorageObject::place(QString filename)
{
    if (isNull()){
        return(false);
    }
    bool noError = false;
    QFileInfo fileInfo(filename);
    QFile file(filename);
    if (fileInfo.exists()){
        if (file.open(QIODevice::ReadOnly)){
            QByteArray byteArray = file.readAll();

            QString azureName = fileInfo.fileName().prepend("TestFolder/");
            if (IoTHubClient_LL_UploadToBlob(iotHubClientHandle, (const char *)azureName.toLocal8Bit().constData(), (const unsigned char *)byteArray.data(), byteArray.length()) != IOTHUB_CLIENT_OK) {
                errorString = QString("%1 failed to upload").arg(filename);
            }
            else {
                noError = true; //file has been successfully uploaded
            }
        file.close();
        }
        else {
            errorString = QString("The file could not be opened successfully!");
        }
    }
    else {
        errorString = QString("The file doesn't exist!");
    }
    return(noError);
}
