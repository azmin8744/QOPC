#include "mainwindow.h"
#include <QApplication>
#include <qopc.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    QOPC *qopc = new QOPC;
    qopc->getConnectMode();

    return a.exec();
}
