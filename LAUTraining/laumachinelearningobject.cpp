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

#include "laumachinelearningobject.h"
#include <iostream>
#include <math.h>
#include <string>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUMachineLearningWidget::onTestButton_clicked()
{
    ;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMachineLearningWidget::LAUMachineLearningWidget(QList<LAUMachineLearningObject::Packet> pckts, LAUMachineLearningObject::MachineLearningClass cls, QWidget *parent) : QWidget(parent)
{
    object = new LAUMachineLearningObject(pckts, cls);
    initialize();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMachineLearningWidget::LAUMachineLearningWidget(QString fileString, QWidget *parent) : QWidget(parent)
{
    object = new LAUMachineLearningObject(fileString);
    initialize();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUMachineLearningWidget::initialize()
{
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(6, 6, 6, 6);

    table = new QTableWidget();
    this->layout()->addWidget(table);

    QVector<QVector<float> > errorMatrix = object->error();
    int numRows = errorMatrix.count();
    if (numRows > 0) {
        int numCols = errorMatrix[0].count();

        table->setRowCount(numRows);
        table->setColumnCount(numCols);
        for (int r = 0; r < numRows; r++) {
            for (int c = 0; c < numCols; c++) {
                QTableWidgetItem *item = new QTableWidgetItem(QString("%1").arg(errorMatrix[r][c] * 100.0f, 5, 'f', 4, '0'));
                int intensity = 255 - errorMatrix[r][c] * 64;
                item->setBackground(QBrush(QColor(255, intensity, intensity)));
                table->setItem(r, c, item);
            }
        }
        table->setHorizontalHeaderLabels(object->classStrings());
        table->setVerticalHeaderLabels(object->classStrings());
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMachineLearningWidget::~LAUMachineLearningWidget()
{
    delete object;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMachineLearningObject::LAUMachineLearningObject(QString fileString, QObject *parent) : QObject(parent), mlClass(ClassNone)
{
    load(fileString);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUMachineLearningObject::LAUMachineLearningObject(QList<Packet> pckts, MachineLearningClass cls, QObject *parent) : QObject(parent), packets(pckts), mlClass(cls)
{
    // CHOOSE BETWEEN A BINARY DECISION TREE OR A MULTI-CLASS DECISION FOREST
    if (mlClass == ClassDT && packets.count() > 2) {
        mlClass = ClassRT;
    } else if (mlClass == ClassRT && packets.count() == 2) {
        mlClass = ClassDT;
    }

    // NOW TRAIN THE SVM AND TEST THE SVM
    train();

    // SAVE A COPY TO THE APPLICATIONS PRIVATE PREFERENCES DIRECTORY
    QString string = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QStringList folders = string.split("/");
    string = QString();
    for (int n = 0; n < folders.count(); n++) {
        string.append(folders.at(n)).append("/");
        if (QDir().exists(string)) {
            ;
        } else {
            QDir().mkdir(string);
        }
    }
    string.append(QString("/machineAI"));
    save(string);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QStringList LAUMachineLearningObject::classStrings() const
{
    QStringList strings;
    for (int n = 0; n < packets.count(); n++) {
        strings << packets.at(n).className;
    }
    return (strings);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUMachineLearningObject::load(QString string)
{
    // GET A FILE TO OPEN FROM THE USER IF NOT ALREADY PROVIDED ONE
    if (string.isNull()) {
        QSettings settings;
        QString directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        directory = settings.value(QString("LAUMachineLearningObject::lastSaveDirectory"), directory).toString();
        string = QFileDialog::getOpenFileName(0, QString("Load machine learning object from disk..."), directory, QString("*.xml;*.dat"));
        if (!string.isNull()) {
            settings.setValue(QString("LAUMachineLearningObject::lastSaveDirectory"), QFileInfo(string).absolutePath());
        } else {
            return (false);
        }
    }

    // SAVE THE FILESTRING FOR LATER
    filestring = string;

    // FIND OUT IF STRING ALREADY HAS ITS EXTENSION AND REMOVE IT SO WE CAN ADD NEW ONES
    if (string.endsWith(".xml") || string.endsWith(".dat")) {
        string.chop(4);
    }

    QString datString = QString("%1.dat").arg(string);
    QFile file(datString);
    if (file.open(QIODevice::ReadOnly)) {
        // GET CLASS OF AI
        QString classString = QString(file.readLine());
        classString.remove("\r");
        classString.remove("\n");
        if (classString == QString("ClassSVM")) {
            mlClass = ClassSVM;
        } else if (classString == QString("ClassDT")) {
            mlClass = ClassDT;
        } else if (classString == QString("ClassRT")) {
            mlClass = ClassRT;
        } else if (classString == QString("ClassBayes")) {
            mlClass = ClassBayes;
        } else if (classString == QString("ClassKNN")) {
            mlClass = ClassKNN;
        } else if (classString == QString("ClassMLP")) {
            mlClass = ClassMLP;
        }

        // GET NUMBER OF MEALS
        int numClasses = QString(file.readLine()).toInt();

        // READ IN MEAL STRINGS FROM DISK
        for (int n = 0; n < numClasses; n++) {
            classString = QString(file.readLine());
            classString.remove("\r");
            classString.remove("\n");

            Packet packet;
            packet.className = classString;
            packets << packet;
        }

        // CREATE ERROR MATRIX TO HOLD VALUES
        errorMatrix.resize(numClasses);
        for (int r = 0; r < numClasses; r++) {
            errorMatrix[r].resize(numClasses);
            for (int c = 0; c < numClasses; c++) {
                errorMatrix[r][c] = QString(file.readLine()).toFloat();
            }
        }

        // WRITE OUT ERROR MATRIX TO DISK
        file.close();
    } else {
        qDebug() << "Unable to open supplied AI file.";
        qDebug() << "Unable to open supplied AI file.";
        qDebug() << "Unable to open supplied AI file.";
    }

    QString xmlString = QString("%1.xml").arg(string);
    switch (mlClass) {
        case ClassSVM:
            svm = cv::Algorithm::load<cv::ml::SVM>(xmlString.toStdString());
            break;
        case ClassDT:
            dtree = cv::Algorithm::load<cv::ml::DTrees>(xmlString.toStdString());
            break;
        case ClassRT:
            rtree = cv::Algorithm::load<cv::ml::RTrees>(xmlString.toStdString());
            break;
        case ClassBayes:
            bayes = cv::Algorithm::load<cv::ml::NormalBayesClassifier>(xmlString.toStdString());
            break;
        case ClassKNN:
            this->loadKNN(xmlString);
            break;
        case ClassMLP:
            mlp = cv::Algorithm::load<cv::ml::ANN_MLP>(xmlString.toStdString());
            break;
    }

    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUMachineLearningObject::save(QString string)
{
    // GET A FILE TO OPEN FROM THE USER IF NOT ALREADY PROVIDED ONE
    if (string.isNull()) {
        QSettings settings;
        QString directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        directory = settings.value(QString("LAUMachineLearningObject::lastSaveDirectory"), directory).toString();
        switch (mlClass) {
            case ClassSVM:
                directory.append(QString("/supportVectorMachine.xml"));
                break;
            case ClassDT:
            case ClassRT:
                directory.append(QString("/decisionTree.xml"));
                break;
            case ClassBayes:
                directory.append(QString("/bayesClassifier.xml"));
                break;
            case ClassKNN:
                directory.append(QString("/nearestNeighbor.xml"));
                break;
            case ClassMLP:
                directory.append(QString("/multiLevelPerceptron.xml"));
                break;
        }
        string = QFileDialog::getSaveFileName(0, QString("Save machine learning object to disk..."), directory, QString("*.xml"));
        if (!string.isNull()) {
            settings.setValue(QString("LAUMachineLearningObject::lastSaveDirectory"), QFileInfo(string).absolutePath());
        } else {
            return (false);
        }
    }

    // SAVE THE FILESTRING FOR LATER
    filestring = string;

    // FIND OUT IF STRING ALREADY HAS ITS AND REMOVE IT SO WE CAN ADD NEW ONES
    if (string.endsWith(".xml") || string.endsWith(".dat") || string.endsWith(".csv")) {
        string.chop(4);
    }

    QString xmlString = QString("%1.xml").arg(string);
    switch (mlClass) {
        case ClassSVM:
            svm->save(xmlString.toStdString());
            break;
        case ClassDT:
            dtree->save(xmlString.toStdString());
            break;
        case ClassRT:
            rtree->save(xmlString.toStdString());
            break;
        case ClassBayes:
            bayes->save(xmlString.toStdString());
            break;
        case ClassKNN:
            this->saveKNN(xmlString);
            break;
        case ClassMLP:
            mlp->save(xmlString.toStdString());
            break;
    }

    QStringList strings = classStrings();
    if (strings.count() > 0) {
        QString datString = QString("%1.dat").arg(string);
        QFile file(datString);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);

            // WRITE OUT CLASS OF OBJECT
            switch (mlClass) {
                case ClassSVM:
                    stream << QString("ClassSVM") << QString("\r\n");
                    break;
                case ClassDT:
                    stream << QString("ClassDT") << QString("\r\n");
                    break;
                case ClassRT:
                    stream << QString("ClassRT") << QString("\r\n");
                    break;
                case ClassBayes:
                    stream << QString("ClassBayes") << QString("\r\n");
                    break;
                case ClassKNN:
                    stream << QString("ClassKNN") << QString("\r\n");
                    break;
                case ClassMLP:
                    stream << QString("ClassMLP") << QString("\r\n");
                    break;
            }

            // WRITE OUT NUMBER OF MEALS
            stream << strings.count() << QString("\r\n");

            // WRITE OUT MEAL STRINGS TO DISK
            for (int n = 0; n < strings.count(); n++) {
                stream << strings.at(n) << QString("\r\n");
            }

            if (errorMatrix.count() > 0) {
                // WRITE OUT ERROR MATRIX
                for (int r = 0; r < errorMatrix.count(); r++) {
                    for (int c = 0; c < errorMatrix[r].count(); c++) {
                        stream << errorMatrix[r][c] << QString("\r\n");
                    }
                }
            }

            // WRITE OUT ERROR MATRIX TO DISK
            file.close();
        }
    }

    if (strings.count() > 0) {
        QString datString = QString("%1.csv").arg(string);
        QFile file(datString);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);

            // WRITE OUT MEAL STRINGS TO HEADER ROW
            for (int n = 0; n < strings.count(); n++) {
                stream << QString(",") << strings.at(n);
            }
            stream << QString("\r\n");

            if (errorMatrix.count() > 0) {
                // WRITE OUT ERROR MATRIX
                for (int r = 0; r < errorMatrix.count() && r < strings.count(); r++) {
                    stream << strings.at(r);
                    for (int c = 0; c < errorMatrix[r].count(); c++) {
                        stream << QString(",") << errorMatrix[r][c] * 100.0f;
                    }
                    stream << QString("\r\n");
                }
            }

            // WRITE OUT ERROR MATRIX TO DISK
            file.close();
        }
    }

    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUMachineLearningObject::saveKNN(QString string)
{
    QFile file(string);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        for (int n = 0; n < packets.count(); n++) {
            stream << packets.at(n).className << ", " << packets.at(n).measurements.count() << ", " << packets.at(n).measurements.first().count() << "\r\n";
            for (int c = 0; c < packets.at(n).measurements.count(); c++) {
                QVector<float> measurements = packets.at(n).measurements.at(c);
                for (int k = 0; k < measurements.length(); k++) {
                    if (k < (measurements.length() - 1)) {
                        stream << measurements[k] << ",";
                    } else {
                        stream << measurements[k] << "\r\n";
                    }
                }
            }
        }
        file.close();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUMachineLearningObject::loadKNN(QString string)
{
    QFile file(string);
    if (file.open(QIODevice::ReadOnly)) {
        while (file.atEnd() == false) {
            // GRAB PACKET HEADER
            QStringList strings = QString(file.readLine().simplified()).split(",");

            bool okay;
            Packet packet;
            packet.className = strings.takeFirst();
            int numMeasurements = strings.takeFirst().toInt(&okay);
            if (okay) {
                // LOAD MEASUREMENT VECTORS FOR THE CURRENT PACKET
                for (int n = 0; n < numMeasurements; n++) {
                    QStringList strings = QString(file.readLine().simplified()).split(",");
                    QVector<float> measurement(strings.count(), 0.0f);
                    for (int c = 0; c < strings.count(); c++) {
                        measurement[c] = strings.at(c).toFloat();
                    }
                    packet.measurements << measurement;
                }
            }
            packets << packet;
        }
        file.close();

        // TRAIN THE KNN TREE
        train();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QString LAUMachineLearningObject::identify(QVector<float> measurement, int *index)
{
    // LETS COPY THE QT DATA STRUCTURE TO A CV::MAT STRUCTURE
    cv::Mat data(1, measurement.count(), CV_32FC1);
    for (int n = 0; n < measurement.count(); n++) {
        data.at<float>(0, n) = measurement[n];
    }

    // FIND OUT HOW EACH SAMPLE LANDS WITHIN THE ERROR MATRIX
    int id = -1;
    int numClasses = packets.count();
    switch (mlClass) {
        case ClassSVM:
            id = qMax(qMin(qRound(svm->predict(data.row(0))), numClasses), -1);
            break;
        case ClassDT:
            id = qMax(qMin(qRound(dtree->predict(data)), numClasses), -1);
            break;
        case ClassRT:
            id = qMax(qMin(qRound(rtree->predict(data)), numClasses), -1);
            break;
        case ClassBayes:
            id = qMax(qMin(qRound(bayes->predict(data)), numClasses), -1);
            break;
        case ClassKNN:
            id = qMax(qMin(qRound(knn->predict(data)), numClasses), -1);
            break;
        case ClassMLP:
            cv::Mat response(1, 1, CV_32FC1);
            mlp->predict(data, response);
            id = qMax(qMin(qRound(response.at<float>(0, 0)), numClasses), -1);
            break;
    }

    // LET REPLACE THE OUTPUT INDEX WITH THE MEAL STRING AND RETURN
    if (index) {
        *index = id;
    }
    return (packets.at(id).className);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QString LAUMachineLearningObject::identify(float *measurement, int length, int *index)
{
    // LETS COPY THE QT DATA STRUCTURE TO A CV::MAT STRUCTURE
    static cv::Mat data(1, length, CV_32FC1);
    memcpy(data.data, measurement, length * sizeof(float));

    // FIND OUT HOW EACH SAMPLE LANDS WITHIN THE ERROR MATRIX
    int id = -1;
    int numClasses = packets.count();
    switch (mlClass) {
        case ClassNone:
            break;
        case ClassSVM:
            id = qMax(qMin(qRound(svm->predict(data)), numClasses), -1);
            break;
        case ClassDT:
            id = qMax(qMin(qRound(dtree->predict(data)), numClasses), -1);
            break;
        case ClassRT:
            id = qMax(qMin(qRound(rtree->predict(data)), numClasses), -1);
            break;
        case ClassBayes:
            id = qMax(qMin(qRound(bayes->predict(data)), numClasses), -1);
            break;
        case ClassKNN:
            id = qMax(qMin(qRound(knn->predict(data)), numClasses), -1);
            break;
        case ClassMLP:
            cv::Mat response(1, 1, CV_32FC1);
            mlp->predict(data, response);
            id = qMax(qMin(qRound(response.at<float>(0, 0)), numClasses), -1);
            break;
    }

    // LET REPLACE THE OUTPUT INDEX WITH THE MEAL STRING AND RETURN
    if (index) {
        *index = id;
    }
    return (packets.at(id).className);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUMachineLearningObject::train()
{
    // GET THE SIZE OF THE TRAINING DATA MATRIX FROM THE AVAILABLE PACKETS
    int rows = 0;
    int cols = 0;
    for (int n = 0; n < packets.count(); n++) {
        Packet packet = packets.at(n);

        // FIND OUT WHAT IS THE MINIMUM NUMBER OF COLOR SAMPLES
        for (int n = 0; n < packet.measurements.count(); n++) {
            cols = qMax(cols, packet.measurements.at(n).count());
            rows++;
        }
    }

    // AT THIS POINT, WE KNOW HOW MANY ROWS AND COLUMNS OF DATA WE HAVE
    cv::Mat input(rows, cols, CV_32FC1);
    cv::Mat otput(rows, 1, CV_32S);

    int row = 0;
    for (int n = 0; n < packets.count(); n++) {
        Packet packet = packets.at(n);

        // COPY TRAINING VECTORS TO OUR TRAINING INPUT MATRIX
        for (int k = 0; k < packet.measurements.count(); k++) {
            for (int col = 0; col < cols; col++) {
                input.at<float>(row, col) = packet.measurements.at(k)[col];
            }
            otput.at<int>(row, 0) = n;
            row++;
        }
    }

    // LETS USE 1/4TH OF THEM FOR TRAINING AND THE REST FOR VALIDATION
    int downsample = 4;
    cv::Mat inTrain(rows / downsample, cols, CV_32FC1);
    cv::Mat otTrain(rows / downsample, 1, CV_32S);
    for (int row = 0; row < inTrain.rows; row++) {
        input.row(downsample * row).copyTo(inTrain.row(row));
        otput.row(downsample * row).copyTo(otTrain.row(row));
    }

    if (mlClass == ClassDT) {
        cv::Mat var_type(inTrain.cols + otTrain.cols, 1, CV_8UC1);
        for (int n = 0; n < var_type.rows; n++) {
            if (n < inTrain.cols) {
                var_type.at<unsigned char>(n, 0) = cv::ml::VAR_NUMERICAL;
            } else {
                var_type.at<unsigned char>(n, 0) = cv::ml::VAR_CATEGORICAL;
            }
            qDebug() << var_type.at<unsigned char>(n, 0);
        }
        cv::ml::TrainData *trainData = cv::ml::TrainData::create(inTrain, cv::ml::ROW_SAMPLE, otTrain, cv::Mat(), cv::Mat(), cv::Mat(), var_type);
        dtree = cv::ml::DTrees::create();
        dtree->train(trainData);
        delete trainData;
    } else if (mlClass == ClassRT) {
        cv::Mat var_type(inTrain.cols + otTrain.cols, 1, CV_8UC1);
        for (int n = 0; n < var_type.rows; n++) {
            if (n < inTrain.cols) {
                var_type.at<unsigned char>(n, 0) = cv::ml::VAR_NUMERICAL;
            } else {
                var_type.at<unsigned char>(n, 0) = cv::ml::VAR_CATEGORICAL;
            }
            qDebug() << var_type.at<unsigned char>(n, 0);
        }
        // TRAIN THE DECISION TREE
        cv::ml::TrainData *trainData = cv::ml::TrainData::create(inTrain, cv::ml::ROW_SAMPLE, otTrain, cv::Mat(), cv::Mat(), cv::Mat(), var_type);
        rtree = cv::ml::RTrees::create();
        rtree->train(trainData);
        delete trainData;
    } else if (mlClass == ClassSVM) {
        svm = cv::ml::SVM::create();
        if (svm) {
            svm->setType(CvSVM::NU_SVC);
            svm->setKernel(CvSVM::RBF);
            svm->setDegree(0);
            svm->setGamma(10.0);
            svm->setCoef0(0);
            svm->setC(2);
            svm->setNu(0.5);
            svm->setP(0.0);

            svm->setNu(0.1);
            svm->setDegree(4);
            svm->setGamma(1.0);

            svm->setClassWeights(cv::Mat());
            svm->setTermCriteria(cv::TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 100000, 1e-6));

            // TRAIN THE DECISION SVN
            svm->train(inTrain, cv::ml::ROW_SAMPLE, otTrain);
        }
    } else if (mlClass == ClassBayes) {
        bayes = cv::ml::NormalBayesClassifier::create();
        bayes->train(inTrain, cv::ml::ROW_SAMPLE, otTrain);
    } else if (mlClass == ClassKNN) {
        knn = cv::ml::KNearest::create();
        knn->setDefaultK(32);
        knn->setIsClassifier(true);
        knn->train(inTrain, cv::ml::ROW_SAMPLE, otTrain);
    } else if (mlClass == ClassMLP) {
        cv::Mat layers = cv::Mat(4, 1, CV_32SC1);

        layers.row(0) = cv::Scalar(inTrain.cols);
        layers.row(1) = cv::Scalar(5 * inTrain.cols);
        layers.row(2) = cv::Scalar(inTrain.cols);
        layers.row(3) = cv::Scalar(1);

        CvTermCriteria criteria;
        criteria.max_iter = 1000;
        criteria.epsilon = 0.00001f;
        criteria.type = CV_TERMCRIT_ITER | CV_TERMCRIT_EPS;

        mlp = cv::ml::ANN_MLP::create();
        mlp->setTrainMethod(cv::ml::ANN_MLP::BACKPROP);
        mlp->setBackpropWeightScale(0.05f);
        mlp->setBackpropMomentumScale(0.05f);
        mlp->setTermCriteria(criteria);
        mlp->setLayerSizes(layers);
        mlp->create();

        // train
        otTrain.convertTo(otTrain, CV_32FC1);
        mlp->train(inTrain, cv::ml::ROW_SAMPLE, otTrain);
    }

    // CREATE A 2D ARRAY TO KEEP TRACK OF CROSS OVER ERRORS
    int numClasses = packets.count();
    errorMatrix.resize(numClasses);
    for (int n = 0; n < numClasses; n++) {
        errorMatrix[n] = QVector<float>(numClasses, 0.0f);
    }

    // FIND OUT HOW EACH SAMPLE LANDS WITHIN THE ERROR MATRIX
    for (int i = 0; i < input.rows; i++) {
        int inClass = otput.at<int>(i, 0);
        int otClass = -1;
        switch (mlClass) {
            case ClassSVM:
                otClass = qMax(qMin(qRound(svm->predict(input.row(i))), numClasses - 1), 0);
                break;
            case ClassDT:
                otClass = qMax(qMin(qRound(dtree->predict(input.row(i))), numClasses - 1), 0);
                break;
            case ClassRT:
                otClass = qMax(qMin(qRound(rtree->predict(input.row(i))), numClasses - 1), 0);
                break;
            case ClassBayes:
                otClass = qMax(qMin(qRound(bayes->predict(input.row(i))), numClasses - 1), 0);
                break;
            case ClassKNN:
                otClass = qMax(qMin(qRound(knn->predict(input.row(i))), numClasses - 1), 0);
                break;
            case ClassMLP:
                cv::Mat response(1, 1, CV_32FC1);
                mlp->predict(input.row(i), response);
                otClass = qMax(qMin(qRound(response.at<float>(0, 0)), numClasses - 1), 0);
                break;
        }
        errorMatrix[inClass][otClass] += 1.0f;
    }

    // DIVIDE THE ERROR MATRIX VALUES BY THE TOTAL NUMBER OF SAMPLES TO GET PERCENTAGES
    for (int m = 0; m < numClasses; m++) {
        int numSamples = 0;
        for (int n = 0; n < numClasses; n++) {
            numSamples += errorMatrix[m][n];
        }
        for (int n = 0; n < numClasses; n++) {
            errorMatrix[m][n] /= (float)numSamples;
        }
    }
}
