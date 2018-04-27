#ifndef LAUCLASSIFIERWIDGET_H
#define LAUCLASSIFIERWIDGET_H

#include <QDebug>
#include <QWidget>
#include <QMenuBar>
#include <QSettings>
#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QTableWidget>
#include <QOpenGLWidget>
#include <QStandardPaths>
#include <QProgressDialog>
#include <QDialogButtonBox>

#include "laumemoryobject.h"
#include "lauvideoglwidget.h"
#include "laumachinelearningobject.h"

class LAUClassifyDialog : public QDialog
{
    Q_OBJECT

public:
    LAUClassifyDialog(QString string = QString(), QWidget *parent = 0);
    ~LAUClassifyDialog();

    bool save(QString string = QString());
    bool isValid() const
    {
        return (object.isValid());
    }
    bool isNull() const
    {
        return (object.isNull());
    }
    void addFiles(QStringList list = QStringList());

protected:
    void accept();

public slots:
    void onCellChanged(int row, int col, int prow, int pcol);
    void onCellDoubleClicked(int row, int col);
    void onTrainButtonClicked();
    void onGoodButtonClicked();
    void onBadButtonClicked();

    void onActionLoadCSVFileFromDisk();
    void onActionLoadFiles();
    void onActionSaveAs();
    void onActionSave();

private:
    typedef struct Packet {
        QString filename;
        int directory;
    } Packet;

    bool modifiedFlag;
    QMenuBar *menuBar;
    QString saveFileName;
    QTableWidget *tableWidget;
    LAUVideoGLWidget *glWidget;
    LAUMemoryObject object;
    QList<Packet> packets;

    void initializeInterface();
    QVector<float> extractFeatures(LAUMemoryObject object);

signals:
    void emitFilenameChanged(QString string);
};

#endif // LAUCLASSIFIERWIDGET_H
