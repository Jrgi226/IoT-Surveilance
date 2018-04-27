/*********************************************************************************
 *                                                                               *
 * Copyright (c) 2017, Dr. Daniel L. Lau                                         *
 * All rights reserved.                                                          *
 *                                                                               *
 * Redistribution and use in source and binary forms, with or without            *
 * modification, are permitted provided that the following conditions are met:   *
 * 1. Redistributions of source code must retain the above copyright             *
 *    notice, this list of conditions and the following disclaimer.              *
 * 2. Redistributions in binary form must reproduce the above copyright          *
 *    notice, this list of conditions and the following disclaimer in the        *
 *    documentation and/or other materials provided with the distribution.       *
 * 3. All advertising materials mentioning features or use of this software      *
 *    must display the following acknowledgement:                                *
 *    This product includes software developed by the <organization>.            *
 * 4. Neither the name of the <organization> nor the                             *
 *    names of its contributors may be used to endorse or promote products       *
 *    derived from this software without specific prior written permission.      *
 *                                                                               *
 * THIS SOFTWARE IS PROVIDED BY Dr. Daniel L. Lau ''AS IS'' AND ANY              *
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED     *
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE        *
 * DISCLAIMED. IN NO EVENT SHALL Dr. Daniel L. Lau BE LIABLE FOR ANY             *
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES    *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;  *
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND   *
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                  *
 *                                                                               *
 *********************************************************************************/

#ifndef LAUMACHINELEARNINGOBJECT_H
#define LAUMACHINELEARNINGOBJECT_H

#include <QFile>
#include <QList>
#include <QDebug>
#include <QVector>
#include <QObject>
#include <QMatrix>
#include <QtGlobal>
#include <QSettings>
#include <QFileDialog>
#include <QStringList>
#include <QTextStream>
#include <QStandardPaths>
#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QDialogButtonBox>

#include "opencv2/opencv.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/core/cvstd.hpp"

#define TESTDETECTION

#define CvSVM                       cv::ml::SVM
#define CvANN_MLP                   cv::ml::ANN_MLP
#define CvRTrees                    cv::ml::RTrees
#define CvDTree                     cv::ml::DTrees
#define CvKNearest                  cv::ml::KNearest
#define CvNormalBayesClassifier     cv::ml::NormalBayesClassifier

class LAUMachineLearningObject;
class LAUMachineLearningWidget;
class LAUMachineLearningDialog;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUMachineLearningObject : public QObject
{
    Q_OBJECT

public:
    typedef struct {
        QString className;
        QList<QVector<float> > measurements;
    } Packet;

    enum MachineLearningClass { ClassNone, ClassSVM, ClassRT, ClassDT, ClassBayes, ClassKNN, ClassMLP };

    explicit LAUMachineLearningObject(QString fileString, QObject *parent = 0);
    explicit LAUMachineLearningObject(QList<Packet> pckts, MachineLearningClass cls = ClassRT, QObject *parent = 0);

    QStringList classStrings() const;
    QString filename() const
    {
        return (filestring);
    }
    QString classString(int index) const
    {
        return (packets.at(index).className);
    }
    QString identify(QVector<float> measurement, int *index = NULL);
    QString identify(float *measurement, int length, int *index = NULL);
    QVector<QVector<float> > error() const
    {
        return (errorMatrix);
    }
    int numClasses() const
    {
        return (packets.count());
    }
    int numDimensions() const
    {
        switch (mlClass) {
            case ClassSVM:
                return (svm->getVarCount());
            case ClassKNN:
                return (knn->getVarCount());
            default:
                return (-1);
        }
        return (-1);
    }

    bool load(QString string);
    bool save(QString string);

signals:

public slots:

private:
    QString filestring;
    QList<Packet> packets;
    MachineLearningClass mlClass;
    QVector<QVector<float> > errorMatrix;

    cv::Ptr<CvSVM> svm;
    cv::Ptr<CvANN_MLP> mlp;
    cv::Ptr<CvRTrees> rtree;
    cv::Ptr<CvDTree> dtree;
    cv::Ptr<CvKNearest> knn;
    cv::Ptr<CvNormalBayesClassifier> bayes;

    void train();
    void saveKNN(QString string);
    void loadKNN(QString string);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUMachineLearningWidget : public QWidget
{
    Q_OBJECT

public:
    typedef struct {
        QString string;
        int index;
        int count;
        int volume;
    } ClassPacket;

    explicit LAUMachineLearningWidget(QList<LAUMachineLearningObject::Packet> pckts, LAUMachineLearningObject::MachineLearningClass cls = LAUMachineLearningObject::ClassRT, QWidget *parent = 0);
    explicit LAUMachineLearningWidget(QString fileString, QWidget *parent = 0);
    ~LAUMachineLearningWidget();

    QString filename() const
    {
        return (object->filename());
    }
    bool save(QString string = QString())
    {
        return (object->save(string));
    }
    bool load(QString string = QString())
    {
        return (object->load(string));
    }

public slots:
    void onTestButton_clicked();

private:
    QTableWidget *table;
    LAUMachineLearningObject *object;

    void initialize();

signals:

};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUMachineLearningDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LAUMachineLearningDialog(QList<LAUMachineLearningObject::Packet> pckts, LAUMachineLearningObject::MachineLearningClass cls = LAUMachineLearningObject::ClassRT, QWidget *parent = 0) : QDialog(parent)
    {
        this->setWindowTitle(QString("AI Trainer"));
        this->setLayout(new QVBoxLayout);
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->layout()->setSpacing(6);

        widget = new LAUMachineLearningWidget(pckts, cls);
        widget->layout()->setContentsMargins(0, 0, 0, 0);
        this->layout()->addWidget(widget);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));

        QPushButton *button = new QPushButton(QString("Test"));
        connect(button, SIGNAL(clicked()), widget, SLOT(onTestButton_clicked()));
        buttonBox->addButton(button, QDialogButtonBox::ActionRole);

        this->layout()->addWidget(buttonBox);
    }
    explicit LAUMachineLearningDialog(QString fileString, QWidget *parent = 0) : QDialog(parent)
    {
        this->setLayout(new QVBoxLayout);
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->layout()->setSpacing(6);

        widget = new LAUMachineLearningWidget(fileString);
        widget->layout()->setContentsMargins(0, 0, 0, 0);
        this->layout()->addWidget(widget);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
        connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));

        QPushButton *button = new QPushButton(QString("Test"));
        connect(button, SIGNAL(clicked()), widget, SLOT(onTestButton_clicked()));
        buttonBox->addButton(button, QDialogButtonBox::ActionRole);

        this->layout()->addWidget(buttonBox);
    }

    QString filename() const
    {
        return (widget->filename());
    }

protected:
    void accept()
    {
        if (widget->save()) {
            QDialog::accept();
        }
    }

private:
    LAUMachineLearningWidget *widget;
};

#endif // LAUMACHINELEARNINGOBJECT_H
