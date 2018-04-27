#include "laurfidwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LAURFIDDialog d;
    return d.exec();
}
