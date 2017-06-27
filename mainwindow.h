#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qopc.h>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void liveViewUpdate(QImage frame);
private:
    Ui::MainWindow *ui;
    QOPC qopc;
};

#endif // MAINWINDOW_H
