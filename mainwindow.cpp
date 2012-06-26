#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include "QFileDialog"
#include "SkySDFSSDK.h"
#include "qfile.h"
#include "QDataStream"
#include <QApplication>
#include "QFileDialog"
#include "QDataStream"
#include "QTimer"
#include "QList"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    lineCount = 1;
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(isThreadFinished()));
    test = new testinfo;
    threadList = new QList<workThread*>;
    ui->setupUi(this);
    ui->textEdit->ensureCursorVisible();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::changeTestinfo()
{
    test->result = "false";
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
        connect(thread,SIGNAL(finished()),this,SLOT(threadOver()));
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
    timer->start(100);

}

void MainWindow::threadOver(QString name)
{
    //    ui->textEdit->append(QString::number(lineCount) + " -----> thread       " + name + "      " +test->result);
    //    lineCount++;
    //    qDebug()<<"thread"<<name<<"is stop";

}

void MainWindow::threadOver()
{
    ui->textEdit->append(QString::number(lineCount) + " -----> thread       " +  "      " +test->result);
    lineCount++;
    //    qDebug()<<"thread"<<name<<"is stop";
}

void MainWindow::isThreadFinished()
{ 
    if(threadList->isEmpty())
    {
        timer->stop();
        ui->textEdit->append("test over!");
        qDebug()<<"All Threads are finished";
        lineCount = 1;
    }
    else{
        for(int i=0;i<threadList->length();i++)
        {
            if(threadList->at(i)->isFinished())
            {
                threadOver(threadList->at(i)->name);
                threadList->removeAt(i);
                //                qDebug()<<"xiancehgns"<<threadList->length();
            }
        }
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

void MainWindow::on_readButton_clicked()
{
    changeTestinfo();
    test->testFunc = T_READ;
    test->openMode = O_WRITE;

    createThread();
    runThread();
}

void MainWindow::on_upLocalFile_clicked()
{
    long long allrst = 0;
    QString fileName = QFileDialog::getOpenFileName(
                this,
                QDir::currentPath());
    sky_sdfs_init("config.ini");
    long long tmpfid = sky_sdfs_createfile(fileName.toAscii().constData(),256*1024*1024,1);
    int tmpfd = sky_sdfs_openfile(tmpfid,O_WRITE);
    qDebug()<<"tmpfid="<<tmpfid<<" tmpfd="<<tmpfd;
    if (!fileName.isNull()) {
        QFile file(fileName);
        if(file.open(QIODevice::ReadOnly)){
            char buff[2*1024*1024];
            while(!file.atEnd()){
                int size = file.read(buff,sizeof(buff));
                qDebug()<<"write start";
                int result = sky_sdfs_write(tmpfd,buff,size);
                //                 qDebug()<<result;
                if(result == -1){
                    char name[100];
                    qDebug()<<"ERROR:"<<getlasterror(tmpfd,name,100);
                }
                else{
                    allrst +=result;
                }
                qDebug()<<allrst<<"/"<<file.size();
            }
            sky_sdfs_close(tmpfd);
            file.close();
        }
    }
}


void MainWindow::on_test_87_1_clicked()
{
    sky_sdfs_init("config.ini");
    long long testfid = sky_sdfs_createfile("testfilename",256*1024*1024,1);
    int testfd = sky_sdfs_openfile(testfid + 123,O_READ);
    if(testfd == -1 and testfid != -1){
        ui->textEdit->append(QString::number(lineCount) + " -----> use wrong fileid to open file           test    OK");
        lineCount++;
    }
    else{
        ui->textEdit->append(QString::number(lineCount) + " -----> use wrong fileid to open file           test    FAIL");
        lineCount++;
    }
    sky_sdfs_cleanup();
}

void MainWindow::on_test_87_2_clicked()
{
    sky_sdfs_init("config.ini");
    long long testfid = sky_sdfs_createfile(QString::number(lineCount).toAscii().constData(),256*1024*1024,1);
//    sleep(1000);
    int testfd = sky_sdfs_openfile(testfid,O_WRITE);
    char buff[2*1024*1024];
    int result = sky_sdfs_read(testfd,buff,sizeof(buff));
    qDebug()<<result<<testfd;
    if(result == -1 and testfd != -1){
        char name1[100];
        int errcode = getlasterror(testfd,name1,100);
        qDebug()<<"ERROR:"<<errcode<<name1;
        if(errcode == 2011){
            ui->textEdit->append(QString::number(lineCount) + " -----> use write mode to read file           test    OK");
            lineCount++;
        }
    }
    else{
        ui->textEdit->append(QString::number(lineCount) + " -----> use write mode to read file           test    FAIL");
        lineCount++;
    }
    sky_sdfs_cleanup();
}


void MainWindow::on_test_87_3_clicked()
{
    sky_sdfs_init("config.ini");
    long long testfid = sky_sdfs_createfile("testfilename",256*1024*1024,1);
    int testfd = sky_sdfs_openfile(testfid,O_READ);
    char buff[2*1024*1024];
    int result = sky_sdfs_write(testfd,buff,sizeof(buff));
    qDebug()<<result;
    if(result == -1 and testfd != -1){
        char name1[100];
        int errcode = getlasterror(testfd,name1,100);
        qDebug()<<"ERROR:"<<errcode<<name1;
        if(errcode == 2011){
            ui->textEdit->append(QString::number(lineCount) + " -----> use read mode to write file           test    OK");
            lineCount++;
        }
    }
    else{
        ui->textEdit->append(QString::number(lineCount) + " -----> use read mode to write file           test    FAIL");
        lineCount++;
    }
    sky_sdfs_cleanup();
}


void MainWindow::on_test_90_1_clicked()
{
    sky_sdfs_init("config.ini");
    long long testfid = sky_sdfs_createfile("testfilename",256*1024*1024,1);
    int testfd = sky_sdfs_openfile(testfid,O_WRITE);
    sky_sdfs_close(testfd);
    char buff[2*1024*1024];
    int result = sky_sdfs_write(testfd,buff,sizeof(buff));
    qDebug()<<result;
    if(result == -1 and testfd != -1){
        char name1[100];
        int errcode = getlasterror(testfd,name1,100);
        qDebug()<<"ERROR:"<<errcode<<name1;
        if(errcode == 102){
            ui->textEdit->append(QString::number(lineCount) + " -----> write to the closed file           test    OK");
            lineCount++;
        }
    }
    else{
        ui->textEdit->append(QString::number(lineCount) + " -----> write to the closed file           test    FAIL");
        lineCount++;
    }
    sky_sdfs_cleanup();
}

void MainWindow::on_test_90_2_clicked()
{
    sky_sdfs_init("config.ini");
    long long testfid = sky_sdfs_createfile("testfilename",256*1024*1024,1);
    int testfd = sky_sdfs_openfile(testfid,O_READ);
    sky_sdfs_close(testfd);
    char buff[2*1024*1024];
    int result = sky_sdfs_read(testfd,buff,sizeof(buff));
    qDebug()<<result;
    if(result == -1 and testfd != -1){
        char name1[100];
        int errcode = getlasterror(testfd,name1,100);
        qDebug()<<"ERROR:"<<errcode<<name1;
        if(errcode == 102){
            ui->textEdit->append(QString::number(lineCount) + " -----> read to the closed file           test    OK");
            lineCount++;
        }
        else{
            ui->textEdit->append(QString::number(lineCount) + " -----> read to the closed file           test    FAIL");
            lineCount++;
        }
    }
    sky_sdfs_cleanup();
}
