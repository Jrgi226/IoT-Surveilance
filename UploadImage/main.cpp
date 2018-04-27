#ifdef ENABLEWIDGETS
#include <QApplication>
#else
#include <QCoreApplication>
#endif
#include <QDir>
#include <QProcess>
#include <QFile>
#include <iostream>
#include <QStandardPaths>
#include <QDateTime>
#include <rcrremotestorageobj.h>

int main(int argc, char **argv)
{
 QCoreApplication app (argc, argv);

 RCRAzureIOTStorageObject iotObject;

 if (iotObject.isValid()){
     QDir dir(QString("/home/iot-1/Documents/CowImages")); //QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation).append("/CowImages"));
     qDebug() << "This is where I'm working from: " + dir.absolutePath();
     if (dir.exists()){
         dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
         dir.setSorting(QDir::Name);

         QFileInfoList list = dir.entryInfoList();
         qDebug() << "Bytes    |   Filename" << endl;
         for (int i = 0; i < list.size(); ++i) {
             QString filename = list.at(i).absoluteFilePath();
             qDebug() << QString::number(list.at(i).size()) + " | " + filename;
             if (filename.endsWith(".tif") || filename.endsWith(".tiff")){

                 // This code is Cole trying to save the images as the current time:
                 //QString currtime = QDateTime::currentDateTime().toString("dd_MM_yyyy'_-_'hh_mm_ss_zzz");
                 //QString newname = dir.path().append("/" + currtime + ".tiff");
                 //dir.rename(filename, newname);
                 //qDebug() << "Upload File Name: " + newname;


                 if (iotObject.place(filename)){
                     if(QFile(filename).remove()==false){
                         return(2); // DELETE THE FILE
                     }
                     qDebug() << "File Upload was Successful!!!" << endl;
                 } else {
                     qDebug() << "IoT failed to upload!" << iotObject.error();
                 }
                QFileInfo fileInfo = list.at(i);
                std::cout << qPrintable(QString("%1 %2").arg(fileInfo.size(), 10)
                                                               .arg(fileInfo.fileName()));
                std::cout << std::endl;
             } else {
                 if (QFile(filename).remove() == false){
                     qDebug() << filename << "filename not ending if .tiff OR .tif was not removed!";
                 }
             }
         }
         return 0;
     }
     qDebug() << "Directory was not found!";
 } else {
     qDebug() << "IoT object is not valid!" << iotObject.error();
 }
 return(1);
}
