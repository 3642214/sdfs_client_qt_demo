#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "workthread.h"
#include "SkySDFSSDK.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setMode(int Mode);
    void upFile(int thread,int blockLenth,int copysize,int buff,int fileSize);
    
private slots:
    void isThreadFinished();
    void on_CreatButton_clicked();
    void on_writeButton_clicked();
    void on_openButton_clicked();
    void on_upLocalFile_clicked();
    void on_readButton_clicked();
    void setLog(QString text);


private:
    Ui::MainWindow *ui;
    testinfo* test;
    fileinfo* info;
    QList<workThread*>* threadList;
    void changeTestinfo();
    void createThread();
    void runThread();
    void threadOver(QString name);
    QTimer *timer;
    void btnOn();
    void btnOff();   
    int lineCount;
    long long readFileID;
    bool uploadFile(long long fileFid, QString fileName);

private slots:
    void threadOver();
    void on_test_87_1_clicked();
    void on_test_87_2_clicked();
    void on_test_87_3_clicked();
    void on_test_90_1_clicked();
    void on_test_90_2_clicked();
    void on_readFileButton_clicked();
    void on_upLocalFile_Ex_clicked();
    void on_readFileInfo_clicked();
    void on_deleteFileButton_clicked();
    void textDown();
    void changeValue();
    void on_lockFileButton_clicked();
    void on_unLockFileButton_clicked();
    void on_serachButton_clicked();
    void on_upLittleFileButton_clicked();
    void on_pushButton_clicked();
    void on_test_137_clicked();
 public slots:

};

#endif // MAINWINDOW_H
