#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&qopc, &QOPC::jpgFrameUpdated, this, &MainWindow::liveViewUpdate);
    connect(&qopc, &QOPC::finderInfoUpdated, this, &MainWindow::finderInfoUpdate);
    connect(ui->shutterButton, &QPushButton::clicked, this, &MainWindow::singleShot);
    qopc.negotiate();
}

MainWindow::~MainWindow()
{
    qopc.closeConnection();
    delete ui;
}

void MainWindow::liveViewUpdate(QImage frame)
{
    ui->liveViewFrame->setPixmap(QPixmap::fromImage(frame));
    ui->liveViewFrame->repaint();
}

void MainWindow::finderInfoUpdate(QOPCLiveViewClient::FinderInformation finderInfo)
{
    ui->fNumber->setText(QString("f%1").arg(finderInfo.currentApertureValue / 10));
}

void MainWindow::singleShot()
{
    qopc.singleShot();
}
