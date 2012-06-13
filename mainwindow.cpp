#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include "QFileDialog"
#include "SkySDFSSDK.h"
#include "qfile.h"
#include "QDataStream"
#include <QApplication>
#include "QTimer"
#include "QList"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(isThreadFinished()));
   test = new testinfo;
   threadList = new QList<workThread*>;
   ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::changeTestinfo()
{
    test->blocklength = atoi(ui->lineEdit_2->text().toAscii());
    test->copysize = atoi(ui->lineEdit_3->text().toAscii());
    test->buffsize = atoi(ui->lineEdit_4->text().toAscii());
    test->filesize = atoi(ui->lineEdit_5->text().toAscii());
}

void MainWindow::createThread()
{
    threadList->clear();
    for(int i=1;i<=atoi(ui->lineEdit->text().toAscii());i++)
    {
        workThread *thread = new workThread(QString::number(i),test);
        threadList->push_back(thread);
//        qDebug()<<i;
    }
}

void MainWindow::runThread()
{
    for(int i=0;i<atoi(ui->lineEdit->text().toAscii());i++)
    {

        threadList->at(i)->start();
        qDebug()<<"thread "<<i<<" is start";
    }
    timer->start(1000);

}

void MainWindow::threadOver(QString name)
{
    qDebug()<<"thread"<<name<<"is stop";
}

void MainWindow::isThreadFinished()
{
    int j=0;
    for(int i=0;i<atoi(ui->lineEdit->text().toAscii());i++)
    {
        if(threadList->at(i)->isFinished())
        {
            threadOver(threadList->at(i)->name);
            j++;
        }
    }
    if(j==threadList->length())
    {
        timer->stop();
        qDebug()<<"All Threads are finished";
    }
}

void MainWindow::on_CreatButton_clicked()
{
    changeTestinfo();
    test->testFunc = T_CRTEATE;
    createThread();
    runThread();
}

void MainWindow::on_writeButton_clicked()
{
    changeTestinfo();
    test->testFunc = T_WRITE;
    createThread();
//    threadList->at(12)->start();
    runThread();
}

void MainWindow::on_openButton_clicked()
{
    changeTestinfo();
    test->testFunc = T_OPEN;
    test->openMode = O_WRITE;
    createThread();
    runThread();

    test->openMode = O_READ;
    createThread();
    runThread();
}

void MainWindow::on_pushButton_clicked()
{

}
