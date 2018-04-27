#ifndef RCRREMOTESTORAGEOBJ_H
#define RCRREMOTESTORAGEOBJ_H

#include <QObject>
#include <stdio.h>
#include <stdlib.h>

/* This sample uses the _LL APIs of iothub_client for example purposes.
That does not mean that HTTP only works with the _LL APIs.
Simply changing the using the convenience layer (functions not having _LL)
and removing calls to _DoWork will yield the same results. */

#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "iothub_client_ll.h"
#include "iothub_message.h"
#include "iothubtransporthttp.h"
#include "certs.h"

static const char *deviceID     = "cowtime";
static const char *primaryKey   = "Ri88u9Y+/oiWt1ic2YJZhO0azJ44FWYwKfWZe0Rld/U=";
static const char *secondaryKey = "A3iBKW0evtE6I1/8zsEqpyf+eD6b1GrRvN9S4OQeUCc=";
static const char *connectStringPrimaryKey   = "HostName=FinalCountdown.azure-devices.net;DeviceId=cowtime;SharedAccessKey=Ri88u9Y+/oiWt1ic2YJZhO0azJ44FWYwKfWZe0Rld/U=";
static const char *connectStringSecondaryKey = "HostName=FinalCountdown.azure-devices.net;DeviceId=cowtime;SharedAccessKey=A3iBKW0evtE6I1/8zsEqpyf+eD6b1GrRvN9S4OQeUCc=";
static const char *proxyHost = NULL;
static int proxyPort = 0;

#define HELLO_WORLD "Hello World from IoTHubClient_LL_UploadToBlob"

#include <QFile>
#include <QDebug>
#include <QStandardPaths>
#include <QFileInfo>

#ifdef ENABLEWIDGETS
#include <QMessageBox>
#include <QFileDialog>
#endif

class RCRRemoteStorageObject : public QObject
{
    Q_OBJECT

public:
    RCRRemoteStorageObject(QObject *parent = 0) : QObject(parent) { ; }
    ~RCRRemoteStorageObject() { qDebug() << "~RCRRemoteStorageObj()" ;}

    bool isValid() const { return(errorString.isEmpty()); }
    bool isNull() const { return(!isValid()); }
    QString error() const { return(errorString); }
    virtual bool place(QString filename) = 0;

protected:
    QString errorString;
};

class RCRSecureFTPStorageObject : public RCRRemoteStorageObject
{
    Q_OBJECT

public:
    RCRSecureFTPStorageObject(QObject *parent = 0);
    ~RCRSecureFTPStorageObject() {;}

    bool place(QString filename) {return false;}

private:

};

class RCRAzureIOTStorageObject : public RCRRemoteStorageObject
{
    Q_OBJECT

public:
    RCRAzureIOTStorageObject(QObject *parent = 0);
    ~RCRAzureIOTStorageObject();

    bool place(QString filename);

private:
    IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
};

#endif // RCRREMOTESTORAGEOBJ_H
