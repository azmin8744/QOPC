#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&qopc, &QOPC::jpgFrameUpdated, this, &MainWindow::liveViewUpdate);
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

void MainWindow::singleShot()
{
    qopc.singleShot();
}
