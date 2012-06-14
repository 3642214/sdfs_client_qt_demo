#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "workthread.h"
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
    void isThreadFinished();
    void on_CreatButton_clicked();
    void on_writeButton_clicked();

    void on_openButton_clicked();

    void on_pushButton_clicked();

    void on_readButton_clicked();

private:
    Ui::MainWindow *ui;
    testinfo* test;
    QList<workThread*>* threadList;
    void changeTestinfo();
    void createThread();
    void runThread();
    void threadOver(QString name);
    QTimer *timer;


};

#endif // MAINWINDOW_H
