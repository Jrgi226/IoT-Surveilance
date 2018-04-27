#include "lauclassifierwidget.h"

using namespace libtiff;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUClassifyDialog::LAUClassifyDialog(QString string, QWidget *parent) : QDialog(parent), modifiedFlag(false), menuBar(NULL), glWidget(NULL)
{
    // INITIALIZE THE USER INTERFACE
    initializeInterface();

    // ASK THE USER TO SPECIFY A FILE TO SAVE IF NOT PROVIDED
    if (string.isEmpty()) {
        QSettings settings;
        QString directory = settings.value(QString("LAUClassifyDialog::saveCsvFileDirectory"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        string = QFileDialog::getOpenFileName(0, QString("Load table from disk (*.csv)"), directory, QString("*.csv"));
        if (string.isEmpty() == false) {
            // SAVE THE DIRECTORY FOR FUTURE SAVE AS CALLS
            settings.setValue(QString("LAUClassifyDialog::saveCsvFileDirectory"), string);
        }
    }

    if (string.isEmpty() == false) {
        // SAVE THE FILENAME FOR SAVING LATER
        saveFileName = string;
        this->setWindowTitle(saveFileName);

        QFile file(string);
        if (file.open(QIODevice::ReadOnly)) {
            QString string = QString(file.readAll());
            string.remove("\r");

            QStringList list = string.split("\n");
            int row = 0;
            for (int n = 0; n < list.count(); n++) {
                QString string = list.at(n);
                QStringList strings = string.split(",");
                if (strings.count() == 3) {
                    tableWidget->setRowCount(row + 1);

                    QTableWidgetItem *item = new QTableWidgetItem(strings.first().simplified());
                    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                    item->setFont(QFont(QString("Courier"), 9));
                    tableWidget->setItem(row, 0, item);

                    item = new QTableWidgetItem(strings.at(1).simplified());
                    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                    item->setFont(QFont(QString("Courier"), 9));
                    tableWidget->setItem(row, 1, item);

                    item = new QTableWidgetItem(strings.last().simplified());
                    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                    item->setFont(QFont(QString("Courier"), 9));
                    if (item->text().simplified() == QString("GOOD")) {
                        item->setBackground(QBrush(QColor(128, 255, 128)));
                    } else if (item->text().simplified() == QString("BAD")) {
                        item->setBackground(QBrush(QColor(255, 128, 128)));
                    }
                    tableWidget->setItem(row, 2, item);

                    tableWidget->setRowHeight(row++, 20);
                }
            }
        }
    }
    tableWidget->setCurrentCell(0, 0);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUClassifyDialog::initializeInterface()
{
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(6, 6, 6, 6);
    this->setSizeGripEnabled(true);

    QWidget *widget = new QWidget();
    widget->setLayout(new QHBoxLayout());
    widget->layout()->setContentsMargins(0, 0, 0, 0);

    QPushButton *goodButton = new QPushButton(QString("GOOD"));
    goodButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    widget->layout()->addWidget(goodButton);

    QPushButton *badButton = new QPushButton(QString("BAD"));
    badButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    widget->layout()->addWidget(badButton);
    this->layout()->addWidget(widget);

    tableWidget = new QTableWidget();
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setColumnCount(3);
    tableWidget->setColumnWidth(1, 60);
    tableWidget->setColumnWidth(2, 60);
    tableWidget->setMinimumWidth(420);
    tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(QString("Filename")));
    tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(QString("Frame")));
    tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(QString("Class")));
    tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    this->layout()->addWidget(tableWidget);

    QPushButton *trainButton = new QPushButton(QString("TRAIN"));
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    box->addButton(trainButton, QDialogButtonBox::ActionRole);

    this->layout()->addWidget(box);

    connect(badButton, SIGNAL(clicked()), this, SLOT(onBadButtonClicked()));
    connect(goodButton, SIGNAL(clicked()), this, SLOT(onGoodButtonClicked()));
    connect(trainButton, SIGNAL(clicked()), this, SLOT(onTrainButtonClicked()));
    connect(box->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
    connect(box->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
    connect(tableWidget, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(onCellChanged(int, int, int, int)));
    connect(tableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onCellDoubleClicked(int, int)));

    menuBar = new QMenuBar();
    QMenu *menu = new QMenu(QString("File"));
    menu->addAction(QString("Add Document..."), this, SLOT(onActionLoadCSVFileFromDisk()));
    menu->addAction(QString("Add Files..."), this, SLOT(onActionLoadFiles()));
    menu->addAction(QString("Save As..."), this, SLOT(onActionSaveAs()));
    menu->addAction(QString("Save..."), this, SLOT(onActionSave()));

    menuBar->addMenu(menu);
    //this->layout()->addWidget(menuBar);

    this->layout()->setMenuBar(menuBar);  //addWidget(menu);

    glWidget = new LAUVideoGLWidget();
    ((QVBoxLayout *)(this->layout()))->insertWidget(0, glWidget);
    glWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    glWidget->setMinimumSize(320, 240);
    tableWidget->setFixedHeight(200);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUClassifyDialog::~LAUClassifyDialog()
{
    save("/tmp/table.csv");
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUClassifyDialog::accept()
{
    if (modifiedFlag && tableWidget->rowCount() > 0) {
        int ret = QMessageBox::warning(this, QString("Save to disk..."), QString("Save table to disk before closing?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ret == QMessageBox::Yes) {
            if (save() == false) {
                return;
            }
        }
        modifiedFlag = false;
    }
    QDialog::accept();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUClassifyDialog::save(QString string)
{
    // ASK THE USER TO SPECIFY A FILE TO SAVE IF NOT PROVIDED
    if (string.isEmpty()) {
        QSettings settings;
        QString directory = settings.value(QString("LAUClassifyDialog::saveCsvFileDirectory"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        string = QFileDialog::getSaveFileName(0, QString("Save table to disk (*.csv)"), directory, QString("*.csv"));
        if (string.isEmpty() == false) {
            // SAVE THE DIRECTORY FOR FUTURE SAVE AS CALLS
            settings.setValue(QString("LAUClassifyDialog::saveCsvFileDirectory"), string);
        } else {
            return (false);
        }
    }

    // SAVE THE FILENAME FOR SAVING LATER
    saveFileName = string;
    this->setWindowTitle(saveFileName);

    // CREATE A FILE TO HOLD THE OUTPUT
    QFile file(string);
    if (file.open(QIODevice::WriteOnly)) {
        // CREATE A TEXT STREAM FOR WRITING TO DISK
        QTextStream stream(&file);
        for (int row = 0; row < tableWidget->rowCount(); row++) {
            // WRITE THE FILE STRING AND ITS CLASS TO DISK
            stream << tableWidget->item(row, 0)->text() << ", " << tableWidget->item(row, 1)->text() << ", " << tableWidget->item(row, 2)->text() << "\r\n";
        }
        file.close();
        modifiedFlag = false;
        return (true);
    }
    return (false);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QVector<float> LAUClassifyDialog::extractFeatures(LAUMemoryObject object)
{
    QVector<float> vector(object.width(), 0.0f);

    for (unsigned int row = 0; row < object.height(); row++) {
        unsigned short *buffer = (unsigned short *)object.constScanLine(row);
        for (unsigned int col = 0; col < object.width(); col++) {
            vector[col] += (float)buffer[col] / 65535.0f;
        }
    }
    return (vector);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUClassifyDialog::addFiles(QStringList list)
{
    // SHOW THE USER A DIALOG TO SELECT FILES IF THE INPUT LIST IS EMPTY
    if (list.isEmpty()) {
        QSettings settings;
        QString directory = settings.value(QString("LAUClassifyDialog::LastUsedDirectory"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        list = QFileDialog::getOpenFileNames(0, QString("Load TIFF files from disk (*.tif)"), directory, QString("*.tif"));
        if (list.count() > 0) {
            directory = list.first();
            settings.setValue(QString("LAUClassifyDialog::LastUsedDirectory"), directory);
        } else {
            return;
        }
    }

    // ITERATE THROUGH ENTIRE LIST OF FILENAMES TO GENERATE A LIST OF FILENAMES
    // AND DIRECTORIES WHICH WE CAN LATER USE TO MAP VIDEO FRAMES TO FILES
    for (int n = 0; n < list.count(); n++) {
        QString filename = list.at(n);

        // IF WE HAVE A VALID TIFF FILE, LOAD FROM DISK
        // OTHERWISE TRY TO CONNECT TO SCANNER
        if (QFile::exists(filename)) {
            // OPEN INPUT TIFF FILE FROM DISK
            TIFF *inTiff = TIFFOpen(filename.toLatin1(), "r");
            if (!inTiff) {
                return;
            }
            unsigned short numDirectories = TIFFNumberOfDirectories(inTiff);
            for (unsigned short m = 0; m < numDirectories; m++) {
                TIFFSetDirectory(inTiff, numDirectories);

                // MAKE SURE WE ONLY LOAD MONOCHROME, 16-BITS PER SAMPLES IMAGES
                unsigned short bitsPerSample;
                TIFFGetField(inTiff, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
                if (bitsPerSample == 16) {
                    int row = tableWidget->rowCount();
                    tableWidget->setRowCount(row + 1);

                    QTableWidgetItem *item = new QTableWidgetItem(filename);
                    item->setFont(QFont(QString("Courier"), 9));
                    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                    tableWidget->setItem(row, 0, item);

                    item = new QTableWidgetItem(QString("%1").arg(m));
                    item->setFont(QFont(QString("Courier"), 9));
                    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                    tableWidget->setItem(row, 1, item);

                    item = new QTableWidgetItem(QString("----"));
                    item->setFont(QFont(QString("Courier"), 9));
                    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                    tableWidget->setItem(row, 2, item);

                    tableWidget->setRowHeight(row, 20);
                }
            }
            TIFFClose(inTiff);
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUClassifyDialog::onCellChanged(int row, int col, int prow, int pcol)
{
    Q_UNUSED(col);
    Q_UNUSED(prow);
    Q_UNUSED(pcol);

    // LOAD TIFF IMAGE FROM DISK AND UPLOAD TO GLWIDGET
    QString string = tableWidget->item(row, 0)->text();
    QString filename = tableWidget->item(row, 0)->text();

    // IF WE HAVE A VALID TIFF FILE, LOAD FROM DISK
    if (QFile::exists(filename)) {
        // OPEN INPUT TIFF FILE FROM DISK
        TIFF *inTiff = TIFFOpen(filename.toLatin1(), "r");
        if (!inTiff) {
            return;
        }
        int directory = tableWidget->item(row, 1)->text().toInt();
        LAUMemoryObject object = LAUMemoryObject(inTiff, directory);
        glWidget->setFrame(object);
    }

    emit emitFilenameChanged(string);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUClassifyDialog::onCellDoubleClicked(int row, int col)
{
    Q_UNUSED(col);

    modifiedFlag = true;

    QTableWidgetItem *item = tableWidget->item(row, 2);
    if (item == NULL) {
        item = new QTableWidgetItem(QString("----"));
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        tableWidget->setItem(row, 2, item);
    } else {
        item->setText(QString("----"));
    }
    item->setBackground(QBrush(QColor(255, 255, 255)));
    item->setFont(QFont(QString("Courier"), 9));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUClassifyDialog::onGoodButtonClicked()
{
    modifiedFlag = true;

    int row = tableWidget->currentRow();
    if (row > -1) {
        QTableWidgetItem *item = tableWidget->item(row, 2);
        if (item == NULL) {
            item = new QTableWidgetItem(QString("GOOD"));
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            tableWidget->setItem(row, 2, item);
        } else {
            item->setText(QString("GOOD"));
        }
        item->setBackground(QBrush(QColor(128, 255, 128)));
        item->setFont(QFont(QString("Courier"), 9));

        // LOOK FOR A LATER ROW THAT HASN'T BEEN CLASSIFIED
        for (int n = row; n < tableWidget->rowCount(); n++) {
            QString string = tableWidget->item(n, 1)->text();
            if (string == QString("----")) {
                tableWidget->setCurrentCell(n, 0);
                return;
            }
        }
        // LOOK FOR A PREVIOUS ROW THAT HASN'T BEEN CLASSIFIED
        for (int n = 0; n < row; n++) {
            QString string = tableWidget->item(n, 1)->text();
            if (string == QString("----")) {
                tableWidget->setCurrentCell(n, 0);
                return;
            }
        }
        // MOVE TO THE NEXT ITEM
        tableWidget->setCurrentCell(qMin(row + 1, tableWidget->rowCount() - 1), 0);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUClassifyDialog::onBadButtonClicked()
{
    modifiedFlag = true;

    int row = tableWidget->currentRow();
    if (row > -1) {
        QTableWidgetItem *item = tableWidget->item(row, 2);
        if (item == NULL) {
            item = new QTableWidgetItem(QString("BAD"));
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            tableWidget->setItem(row, 2, item);
        } else {
            item->setText(QString("BAD"));
        }
        item->setBackground(QBrush(QColor(255, 128, 128)));
        item->setFont(QFont(QString("Courier"), 9));

        // LOOK FOR A LATER ROW THAT HASN'T BEEN CLASSIFIED
        for (int n = row; n < tableWidget->rowCount(); n++) {
            QString string = tableWidget->item(n, 2)->text();
            if (string == QString("----")) {
                tableWidget->setCurrentCell(n, 0);
                return;
            }
        }
        // LOOK FOR A PREVIOUS ROW THAT HASN'T BEEN CLASSIFIED
        for (int n = 0; n < row; n++) {
            QString string = tableWidget->item(n, 2)->text();
            if (string == QString("----")) {
                tableWidget->setCurrentCell(n, 0);
                return;
            }
        }
        // MOVE TO THE NEXT ITEM
        tableWidget->setCurrentCell(qMin(row + 1, tableWidget->rowCount() - 1), 0);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUClassifyDialog::onTrainButtonClicked()
{
    if (tableWidget->rowCount() == 0) {
        return;
    }

    // CREATE AN AI PACKET LIST TO HOLD THE TRAINING DATA
    LAUMachineLearningObject::Packet goodPacket;
    goodPacket.className = QString("GOOD");

    LAUMachineLearningObject::Packet badPacket;
    badPacket.className = QString("BAD");

    // ITERATE THROUGH EACH ROW TO LOAD A SCAN AND EXTRACT ITS FEATURE VECTOR
    QProgressDialog dialog(QString("Extracting feature vectors from dataset..."), QString("Abort"), 0, tableWidget->rowCount(), this, Qt::Sheet);
    dialog.show();
    for (int row = 0; row < tableWidget->rowCount(); row++) {
        if (dialog.wasCanceled()) {
            dialog.close();
            return;
        } else {
            dialog.setValue(row);
            qApp->processEvents();
        }

        QString fileString = tableWidget->item(row, 0)->text();
        QString classString = tableWidget->item(row, 2)->text();
        int directory = tableWidget->item(row, 1)->text().toInt();
        if (classString == QString("GOOD")) {
            LAUMemoryObject object(fileString, directory);
            if (object.isValid()) {
                goodPacket.measurements << extractFeatures(object);
            }
        } else if (classString == QString("BAD")) {
            LAUMemoryObject object(fileString, directory);
            if (object.isValid()) {
                badPacket.measurements << extractFeatures(object);
            }
        }
    }
    dialog.close();

    // TRAIN THE AI AND REPORT THE RESULTS ON SCREEN
    QList<LAUMachineLearningObject::Packet> packets;
    packets << goodPacket;
    packets << badPacket;

    // SAVE TRAINING DATA TO DISK IN TEMPORARY LOCATION
    QFile file("/tmp/goodData.csv");
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        for (int row = 0; row < goodPacket.measurements.count(); row++) {
            QVector<float> vector = goodPacket.measurements.at(row);
            for (int col = 0; col < vector.length(); col++) {
                if (col < vector.length() - 1) {
                    stream << vector[col] << ",";
                } else {
                    stream << vector[col] << "\r\n";
                }
            }
        }
        file.close();
    }

    file.setFileName("/tmp/badData.csv");
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        for (int row = 0; row < badPacket.measurements.count(); row++) {
            QVector<float> vector = badPacket.measurements.at(row);
            for (int col = 0; col < vector.length(); col++) {
                if (col < vector.length() - 1) {
                    stream << vector[col] << ",";
                } else {
                    stream << vector[col] << "\r\n";
                }
            }
        }
        file.close();
    }

    LAUMachineLearningDialog aiDialog(packets, LAUMachineLearningObject::ClassSVM, this);
    if (aiDialog.exec() == QDialog::Accepted) {
        // LOAD THE AI BACK FROM DISK
        LAUMachineLearningObject mlObject(aiDialog.filename());

        // CHECK FOR PARSED FOLDERS AND IF THEY DON'T EXIST CREATE THEM
        QString string = mlObject.filename();
        string.truncate(string.lastIndexOf("/"));

        QString ggString = QString("%1/Good/Good/").arg(string);
        QString gbString = QString("%1/Good/Bad/").arg(string);
        QString bbString = QString("%1/Bad/Bad/").arg(string);
        QString bgString = QString("%1/Bad/Good/").arg(string);

        QDir dirGG;
        if (dirGG.exists(ggString) == false) {
            dirGG.mkpath(ggString);
        }
        dirGG.setPath(ggString);
        QDir dirGB;
        if (dirGG.exists(gbString) == false) {
            dirGB.mkpath(gbString);
        }
        dirGB.setPath(gbString);
        QDir dirBB;
        if (dirGG.exists(bbString) == false) {
            dirBB.mkpath(bbString);
        }
        dirBB.setPath(bbString);
        QDir dirBG;
        if (dirGG.exists(bgString) == false) {
            dirBG.mkpath(bgString);
        }
        dirBG.setPath(bgString);

        // ITERATE THROUGH EACH ROW TO LOAD A SCAN AND EXTRACT ITS
        // FEATURE VECTOR AND SAVE THE SCAN TO ITS CLASSIFIED FOLDER
        QProgressDialog dialog(QString("Classifying dataset into separate folders..."), QString("Abort"), 0, tableWidget->rowCount(), this, Qt::Sheet);
        dialog.show();
        for (int row = 0; row < tableWidget->rowCount(); row++) {
            if (dialog.wasCanceled()) {
                dialog.close();
                return;
            } else {
                dialog.setValue(row);
                qApp->processEvents();
            }

            QString filename = tableWidget->item(row, 0)->text();
            QString classString = tableWidget->item(row, 2)->text();
            int directory = tableWidget->item(row, 1)->text().toInt();
            if (classString == QString("GOOD")) {
                LAUMemoryObject object(filename, directory);
                if (object.isValid()) {
                    QString idString = mlObject.identify(extractFeatures(object));
                    if (idString == QString("GOOD")) {
                        QString string = filename.split("/").last().split(".").first();
                        QString filestring = QString("%1/%2_%3.tif").arg(ggString).arg(string).arg(directory);
                        object.save(filestring);
                    } else {
                        QString string = filename.split("/").last().split(".").first();
                        QString filestring = QString("%1/%2_%3.tif").arg(gbString).arg(string).arg(directory);
                        object.save(filestring);
                    }
                }
            } else if (classString == QString("BAD")) {
                LAUMemoryObject object(filename, directory);
                if (object.isValid()) {
                    QString idString = mlObject.identify(extractFeatures(object));
                    if (idString == QString("GOOD")) {
                        QString string = filename.split("/").last().split(".").first();
                        QString filestring = QString("%1/%2_%3.tif").arg(bgString).arg(string).arg(directory);
                        object.save(filestring);
                    } else {
                        QString string = filename.split("/").last().split(".").first();
                        QString filestring = QString("%1/%2_%3.tif").arg(bbString).arg(string).arg(directory);
                        object.save(filestring);
                    }
                }
            }
        }
        dialog.close();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUClassifyDialog::onActionLoadCSVFileFromDisk()
{
    // ASK THE USER TO SPECIFY A FILE TO SAVE IF NOT PROVIDED
    QSettings settings;
    QString directory = settings.value(QString("LAUClassifyDialog::saveCsvFileDirectory"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    QString string = QFileDialog::getOpenFileName(0, QString("Load table from disk (*.csv)"), directory, QString("*.csv"));
    if (string.isEmpty() == false) {
        // SAVE THE DIRECTORY FOR FUTURE SAVE AS CALLS
        settings.setValue(QString("LAUClassifyDialog::saveCsvFileDirectory"), string);

        QFile file(string);
        if (file.open(QIODevice::ReadOnly)) {
            QString string = QString(file.readAll());
            string.remove("\r");

            QStringList list = string.split("\n");
            int row = tableWidget->rowCount();
            for (int n = 0; n < list.count(); n++) {
                QString string = list.at(n);
                QStringList strings = string.split(",");
                if (strings.count() == 3) {
                    tableWidget->setRowCount(row + 1);

                    QTableWidgetItem *item = new QTableWidgetItem(strings.first().simplified());
                    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                    item->setFont(QFont(QString("Courier"), 9));
                    tableWidget->setItem(row, 0, item);

                    item = new QTableWidgetItem(strings.at(1).simplified());
                    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                    item->setFont(QFont(QString("Courier"), 9));
                    tableWidget->setItem(row, 1, item);

                    item = new QTableWidgetItem(strings.last().simplified());
                    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                    item->setFont(QFont(QString("Courier"), 9));
                    if (item->text().simplified() == QString("GOOD")) {
                        item->setBackground(QBrush(QColor(128, 255, 128)));
                    } else if (item->text().simplified() == QString("BAD")) {
                        item->setBackground(QBrush(QColor(255, 128, 128)));
                    }
                    tableWidget->setItem(row, 2, item);

                    tableWidget->setRowHeight(row++, 20);
                }
            }
        }

        // SET THE CURRENT ROW IN ORDER TO FILL THE GLWIDGET
        int index = qMax(tableWidget->currentRow(), 0);
        tableWidget->setCurrentCell(index, 0);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUClassifyDialog::onActionLoadFiles()
{
    addFiles();

    // SET THE CURRENT ROW IN ORDER TO FILL THE GLWIDGET
    int index = qMax(tableWidget->currentRow(), 0);
    tableWidget->setCurrentCell(index, 0);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUClassifyDialog::onActionSaveAs()
{
    save(QString());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUClassifyDialog::onActionSave()
{
    save(saveFileName);
}
