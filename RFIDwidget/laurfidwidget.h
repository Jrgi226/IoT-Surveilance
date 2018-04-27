#ifndef LAUSERIALWIDGET_H
#define LAUSERIALWIDGET_H

#include <QTime>
#include <QDebug>
#include <QWidget>
#include <QDialog>
#include <QTextEdit>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSerialPort>
#include <QInputDialog>
#include <QSerialPortInfo>
#include <QDialogButtonBox>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURFIDObject : public QObject
{
    Q_OBJECT

public:
    LAURFIDObject(QString string = QString(), QObject *parent = 0);
    ~LAURFIDObject();

    bool isValid() const
    {
        return (port.isOpen());
    }

    bool isNull() const
    {
        return (!isValid());
    }

    QString lastRFID() const
    {
        return (rfid);
    }

private:
    QString portString;        // PORT STRING
    QSerialPort port;          // INSTANCE OF THE SERIAL PORT
    QString rfid;

private slots:
    void onReadyRead();

signals:
    void emitError(QString);
    void emitRFID(QString, QTime);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURFIDWidget : public QWidget
{
    Q_OBJECT

public:
    LAURFIDWidget(QString string = QString(), QWidget *parent = 0);
    ~LAURFIDWidget();

public slots:
    void onError(QString string)
    {
        qDebug() << string;
    }

    void onRFID(QString string, QTime time)
    {
        // ADD THE INCOMING RFID TAG TO THE TEXT EDIT WINDOW FOR THE USER TO SEE
        textEdit->append(string);

        // PRINT IN THE APPLICATION OUTPUT THE RFID TAG AND THE TIME IT WAS RECORDED
        qDebug() << string << time.toString(Qt::ISODate);
    }

private:
    QTextEdit *textEdit;
    LAURFIDObject *serial;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAURFIDDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LAURFIDDialog(QString string = QString(), QWidget *parent = 0) : QDialog(parent), widget(NULL)
    {
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->setWindowTitle(QString("RFID Tag Reader"));

        widget = new LAURFIDWidget(string);
        this->layout()->addWidget(widget);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(buttonBox);
    }

private:
    LAURFIDWidget *widget;
};

#endif // LAUSERIALWIDGET_H
