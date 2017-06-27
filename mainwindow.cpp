#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&qopc, &QOPC::jpgFrameUpdated, this, &MainWindow::liveViewUpdate);
    qopc.negotiate();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::liveViewUpdate(QImage frame)
{
    ui->liveViewFrame->setPixmap(QPixmap::fromImage(frame));
    ui->liveViewFrame->repaint();
}
