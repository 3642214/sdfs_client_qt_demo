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
    void on_upLocalFile_clicked();
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
    int lineCount;
    void btnOn();
    void btnOff();
private slots:
    void threadOver();
    void on_test_87_1_clicked();
    void on_test_87_2_clicked();
    void on_test_87_3_clicked();
    void on_test_90_1_clicked();
    void on_test_90_2_clicked();
};

#endif // MAINWINDOW_H
