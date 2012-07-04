#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QDebug"
#include "QFileDialog"
#include "qfile.h"
#include "QDataStream"
#include <QApplication>
#include "QFileDialog"
#include "QDataStream"
#include "QTimer"
#include "QList"


#define BUFFSIZE		1024*1024
#define BLOCKLENGTH     256*1024*1024

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    lineCount = 1;
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(isThreadFinished()));
    test = new testinfo;
    info = new fileinfo;
    threadList = new QList<workThread*>;
    ui->setupUi(this);
    ui->textEdit->ensureCursorVisible();
    sky_sdfs_init("config.ini");
}

MainWindow::~MainWindow()
{
    sky_sdfs_cleanup();
    delete ui;
}


void MainWindow::changeTestinfo()
{
    btnOff();
    test->result = "false";
    test->blocklength = atoi(ui->lineEdit_2->text().toAscii());
    test->copysize = atoi(ui->lineEdit_3->text().toAscii());
    test->buffsize = atoi(ui->lineEdit_4->text().toAscii());
    test->filesize = atoi(ui->lineEdit_5->text().toAscii());
    if(test->filesize < test->buffsize){
        test->buffsize = test->filesize;
        ui->lineEdit_4->setText(QString::number(test->filesize));
    }
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

void MainWindow::btnOn()
{
    ui->widget->setEnabled(true);
    ui->widget_2->setEnabled(true);
}

void MainWindow::btnOff()
{
    ui->widget->setEnabled(false);
    ui->widget_2->setEnabled(false);
}

bool MainWindow::uploadFile(long long fileFid,QString fileName)
{
    long long allrst = 0;
    int fileFd = sky_sdfs_openfile(fileFid,O_WRITE);
    qDebug()<<"fileName= "<<fileName<<" fileFid="<<fileFid<<" fileFd="<<fileFd;
    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly) and fileFid > 0 and fileFd > 0){
        char buff[BUFFSIZE];
        while(!file.atEnd()){
            int size = file.read(buff,sizeof(buff));
            //            qDebug()<<"write start";
            int result = sky_sdfs_write(fileFd,buff,size);
            //                 qDebug()<<result;
            if(result == -1){
                char name[100];
                qDebug()<<"ERROR:"<<getlasterror(fileFd,name,100)<<name;
                return false;
            }
            else{
                allrst +=result;
            }
            qDebug()<<allrst<<"/"<<file.size();
        }
        return true;
    }

    file.close();
    sky_sdfs_close(fileFd);
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
        btnOn();
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

    QString fileName = QFileDialog::getOpenFileName(
                this,
                QDir::currentPath());
    if (!fileName.isNull()) {
        int copies = atoi(ui->lineEdit_3->text().toAscii());
        long long fileFid = sky_sdfs_createfile(fileName.toAscii().constData(),BLOCKLENGTH,copies);
        if(uploadFile(fileFid,fileName)){
            ui->textEdit->append(
                        "fileName= "
                        +  fileName
                        +" fileFid= "
                        +  QString::number(fileFid)
                        + "  upload OK");
        }
        else{
            ui->textEdit->append("upload fail");
        }
    }
}


void MainWindow::on_test_87_1_clicked()
{
    long long testfid = sky_sdfs_createfile("testfilename",BLOCKLENGTH,1);
    int testfd = sky_sdfs_openfile(testfid + 123,O_READ);
    if(testfd == -1 and testfid != -1){
        ui->textEdit->append(QString::number(lineCount) + " -----> use wrong fileid to open file           test    OK");
        lineCount++;
    }
    else{
        ui->textEdit->append(QString::number(lineCount) + " -----> use wrong fileid to open file           test    FAIL");
        lineCount++;
    }
    //    sky_sdfs_cleanup();
}

void MainWindow::on_test_87_2_clicked()
{
    long long testfid = sky_sdfs_createfile(QString::number(lineCount).toAscii().constData(),BLOCKLENGTH,1);
    int testfd = sky_sdfs_openfile(testfid,O_WRITE);
    char buff[BUFFSIZE];
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
    //    sky_sdfs_cleanup();
}


void MainWindow::on_test_87_3_clicked()
{
    //    sky_sdfs_init("config.ini");
    long long testfid = sky_sdfs_createfile("testfilename",BLOCKLENGTH,1);
    int testfd = sky_sdfs_openfile(testfid,O_READ);
    char buff[BUFFSIZE];
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
}


void MainWindow::on_test_90_1_clicked()
{
    long long testfid = sky_sdfs_createfile("testfilename",BLOCKLENGTH,1);
    int testfd = sky_sdfs_openfile(testfid,O_WRITE);
    sky_sdfs_close(testfd);
    char buff[BUFFSIZE];
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
    //    sky_sdfs_cleanup();
}

void MainWindow::on_test_90_2_clicked()
{
    //    sky_sdfs_init("config.ini");
    long long testfid = sky_sdfs_createfile("testfilename",BLOCKLENGTH,1);
    int testfd = sky_sdfs_openfile(testfid,O_READ);
    sky_sdfs_close(testfd);
    char buff[BUFFSIZE];
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
    //    sky_sdfs_cleanup();
}

void MainWindow::on_readFileButton_clicked()
{
    readFileID = atoi(ui->lineEdit_6->text().toAscii());
    int fd = sky_sdfs_openfile(readFileID,O_READ);
    //   qDebug()<<fd;
    QFile testFile("FileId_" + QString("%1").arg(readFileID));

    if(testFile.open(QIODevice::WriteOnly) and fd > 0){
        char buff[BUFFSIZE];
        while(TRUE){
            int result = 0;
            result = sky_sdfs_read(fd,buff,sizeof(buff));
            //                      qDebug()<<"Thread "<<name<<"read= "<<result;
            if (result > 0){
                qint64 writelength = testFile.write(buff,result);
            }
            else{
                if(result == -1){
                    char name1[100];
                    qDebug()<<"ERROR:"<<getlasterror(fd,name1,100)<<name1;
                }
                else{
                    ui->textEdit->append(QString::number(lineCount)+ " ----->" + testFile.fileName() + "  file download OK");
                    lineCount++;
                    break;
                }

            }
        }
        testFile.close();
    }
    else{
        ui->textEdit->append(QString::number(lineCount)+ " ----->" + testFile.fileName() + "  file download Fail");
        lineCount++;
    }
}

void MainWindow::on_upLocalFile_Ex_clicked()
{
    QString videoFile = QFileDialog::getOpenFileName(
                this,
                tr("choose the video file"),
                QDir::currentPath());

    QString idxFile = QFileDialog::getOpenFileName(
                this,
                tr("choose the index file"),
                QDir::currentPath(),
                tr("Index (*.idx)"));
    if (!videoFile.isNull() and !idxFile.isNull()) {
        QString startTime = "2012-06-29 12:12:12.012";
        int copies = atoi(ui->lineEdit_3->text().toAscii());
        long long videoFid = sky_sdfs_createfile_ex(videoFile.toAscii().constData(),
                                                    BLOCKLENGTH,
                                                    copies,
                                                    NORMAL_FILE,
                                                    startTime.toAscii().data(),
                                                    0);

        uploadFile(videoFid,videoFile);
        long long idxFid = sky_sdfs_createfile_ex(idxFile.toAscii().constData(),
                                                  BLOCKLENGTH,
                                                  copies,
                                                  INDEX_FILE,
                                                  startTime.toAscii().data(),
                                                  videoFid);
        uploadFile(idxFid,idxFile);
        ui->textEdit->append("videoFid= "+  QString::number(videoFid) + "    idxFid= " + QString::number(idxFid));
    }

}

void MainWindow::on_readFileInfo_clicked()
{
    readFileID = atoi(ui->lineEdit_6->text().toAscii());
    int res = sky_sdfs_fileinfo(readFileID,info);
    qDebug()<<res
            <<info->beginTime
            <<info->blocklength
            <<info->blocksize
            <<info->copysize
            <<info->fileid
            <<info->filemode
            <<info->filetype
            <<info->link
            <<info->name
            <<info->owner;
    if(res == 0){
        ui->textEdit->append("================ File Info ================");
        ui->textEdit->append("readFileID: " + QString::number(readFileID));
        ui->textEdit->append("beginTime: " + QString::number(info->beginTime));
        ui->textEdit->append("blocklength: " + QString::number(info->blocklength));
        ui->textEdit->append("blocksize: " + QString::number(info->blocksize));
        ui->textEdit->append("copysize: " + QString::number(info->copysize));
        ui->textEdit->append("fileid: " + QString::number(info->fileid));
        ui->textEdit->append("filemode: " + QString::number(info->filemode));
        ui->textEdit->append("filetype: " + QString::number(info->filetype));
        ui->textEdit->append("link: " + QString::number(info->link));
        ui->textEdit->append("name: " + QString(info->name));
        ui->textEdit->append("owner: " + QString::number(info->owner));
        ui->textEdit->append("================ File Info ================");
    }
    else{
       ui->textEdit->append("======= not ================ find =========");
       ui->textEdit->append("readFileID: " + QString::number(readFileID) + "   file not find");
       ui->textEdit->append("======= not ================ find =========");
    }

}
