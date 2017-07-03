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
    // F値
    double fNumber = (double)finderInfo.currentApertureValue / 10;
    ui->fNumber->setText(QString("f%1").arg(fNumber, 3, 'f', 1));

    // SS
    int numerator = finderInfo.currentSSNumerator;      // 分子
    int denominator = finderInfo.currentSSDenominator;  // 分母
    if(numerator == 1)
    {
        ui->shutterSpeed->setText(QString("1/%1").arg(denominator));
    }
    else
    {
        ui->shutterSpeed->setText(QString("%1s").arg(numerator));
    }
}

void MainWindow::singleShot()
{
    qopc.singleShot();
}
