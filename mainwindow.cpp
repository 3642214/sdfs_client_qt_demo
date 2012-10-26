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
#include "QFileInfo"
#include "QDateTime"
#include "QMessageBox"

#define BLOCKLENGTH     256*1024*1024
#define ADVANCE_MODE 1
#define NORMAL_MODE 0

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
    sky_sdfs_init("config.ini");
    ui->groupBox_AutoTest->setEnabled(false);
    ui->groupBox_TestLink->setEnabled(false);
    ui->deleteFileButton->setEnabled(false);
    connect(ui->textEdit,SIGNAL(textChanged()),this,SLOT(textDown()));
}

MainWindow::~MainWindow()
{
    sky_sdfs_cleanup();
    if(!threadList->isEmpty()){
        qDebug()<<threadList->length();
        for(int i=0;i<threadList->length();i++)
        {
//            threadList->at(i)->quit();
            threadList->at(i)->terminate();
            qDebug()<<1;
        }
        qDebug()<<threadList->at(0)->isRunning()<<"2";
    }

    qDebug()<<"31";

    qDebug()<<"3";
    delete ui;
}

void MainWindow::setMode(int Mode)
{
    if(Mode == ADVANCE_MODE){
        ui->groupBox_AutoTest->setEnabled(true);
        ui->groupBox_TestLink->setEnabled(true);
        ui->deleteFileButton->setEnabled(true);
    }
}

void MainWindow::upFile(int thread, int blockLenth, int copysize, int buff, int fileSize)
{
    ui->lineEdit_thread->setText(QString::number(thread));
    ui->lineEdit_blockLenth->setText(QString::number(blockLenth));
    ui->lineEdit_copySize->setText(QString::number(copysize));
    ui->lineEdit_buffSize->setText(QString::number(buff));
    ui->lineEdit_fileSize->setText(QString::number(fileSize));
    setMode(ADVANCE_MODE);
    ui->writeButton->click();
    while(true){
        isThreadFinished();
        if(threadList->isEmpty()){
            exit(0);
        }
    }
}


void MainWindow::changeTestinfo()
{
    btnOff();
    test->result = "false";
    test->blocklength = atoi(ui->lineEdit_blockLenth->text().toAscii());
    test->copysize = atoi(ui->lineEdit_copySize->text().toAscii());
    test->buffsize = atoi(ui->lineEdit_buffSize->text().toAscii());
    test->filesize = atoi(ui->lineEdit_fileSize->text().toAscii());
    if(test->filesize < test->buffsize){
        test->buffsize = test->filesize;
        ui->lineEdit_fileSize->setText(QString::number(test->filesize));
    }
}

void MainWindow::createThread()
{
    threadList->clear();
    for(int i=1;i<=atoi(ui->lineEdit_thread->text().toAscii());i++)
    {
        workThread *thread = new workThread(QString::number(i),test);
        threadList->push_back(thread);
        connect(thread,SIGNAL(finished()),this,SLOT(threadOver()));
        connect(thread,SIGNAL(changeText(QString)),this,SLOT(setLog(QString)));
        //        qDebug()<<i;
    }
}

void MainWindow::runThread()
{
    for(int i=0;i<atoi(ui->lineEdit_thread->text().toAscii());i++)
    {

        threadList->at(i)->start();
        qDebug()<<"thread "<<i<<" is start";
    }
    timer->start(1000);

}

void MainWindow::threadOver(QString name)
{
    //    ui->textEdit->append(QString::number(lineCount) + " -----> thread       " + name + "      " +test->result);
    //    lineCount++;
    //    qDebug()<<"thread"<<name<<"is stop";

}

void MainWindow::btnOn()
{
    ui->groupBox_AutoTest->setEnabled(true);
    ui->groupBox_ManualTest->setEnabled(true);
    ui->groupBox_TestLink->setEnabled(true);
}

void MainWindow::btnOff()
{
    ui->groupBox_AutoTest->setEnabled(false);
    ui->groupBox_ManualTest->setEnabled(false);
    ui->groupBox_TestLink->setEnabled(false);
}

bool MainWindow::uploadFile(long long fileFid,QString fileName)
{
    bool res = false;
    long long allrst = 0;
    int fileFd = sky_sdfs_openfile(fileFid,O_WRITE);
    qDebug()<<"fileName= "<<fileName<<" fileFid="<<fileFid<<" fileFd="<<fileFd;
    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly) and fileFid > 0 and fileFd > 0){
//        char buff[BUFFSIZE];
        int buffSize = atoi(ui->lineEdit_buffSize->text().toAscii());
        char* buff = new char [buffSize*1024*1024];
        while(!file.atEnd()){
            int size = file.read(buff,(buffSize*1024*1024)*sizeof(char));
            //                        qDebug()<<"write start" << size;
            int result = sky_sdfs_write(fileFd,buff,size);
            //                 qDebug()<<result;
            if(result == -1){
                char name[100];
                qDebug()<<"ERROR:"<<getlasterror(fileFd,name,100)<<name;
                break;
            }
            else{
                allrst += result;
            }
            qDebug()<<allrst<<"/"<<file.size();
            if(file.atEnd() and result != -1){
               res = true;
            }
        }
        file.close();
        delete [] buff;
    }
    sky_sdfs_close(fileFd);
    return res;
}

void MainWindow::setLog(QString text)
{
    ui->textEdit->append(text);
}


void MainWindow::threadOver()
{
    //    ui->textEdit->append(QString::number(lineCount) + " -----> thread       " + "      " +test->result);
    //    lineCount++;
    //        qDebug()<<"thread"<<name<<"is stop";
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
//        qDebug()<<threadList->length();
        for(int i=0;i<threadList->length();i++)
        {
            if(threadList->at(i)->isFinished())
            {
                //                threadOver(threadList->at(i)->name);
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
    QList<QString> fileList = QFileDialog::getOpenFileNames(
                this,
                QDir::currentPath());

    if (!fileList.isEmpty()) {
        changeTestinfo();
        ui->selectFileCount->setText(QString::number(fileList.length()));
        test->filePath = fileList;
        test->count = ui->uploadCount->text().toInt();
        test->testFunc = T_UPLOAD;
        createThread();
        runThread();
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
//    char buff[BUFFSIZE];
    int buffSize = atoi(ui->lineEdit_buffSize->text().toAscii());
    char* buff = new char [buffSize*1024*1024];
    int result = sky_sdfs_read(testfd,buff,(buffSize*1024*1024)*sizeof(char));
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
    delete [] buff;
    //    sky_sdfs_cleanup();
}


void MainWindow::on_test_87_3_clicked()
{
    //    sky_sdfs_init("config.ini");
    long long testfid = sky_sdfs_createfile("testfilename",BLOCKLENGTH,1);
    int testfd = sky_sdfs_openfile(testfid,O_READ);
//    char buff[BUFFSIZE];
    int buffSize = atoi(ui->lineEdit_buffSize->text().toAscii());
    char* buff = new char [buffSize*1024*1024];
    int result = sky_sdfs_write(testfd,buff,(buffSize*1024*1024)*sizeof(char));
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
    delete [] buff;
}


void MainWindow::on_test_90_1_clicked()
{
    long long testfid = sky_sdfs_createfile("testfilename",BLOCKLENGTH,1);
    int testfd = sky_sdfs_openfile(testfid,O_WRITE);
    sky_sdfs_close(testfd);
//    char buff[BUFFSIZE];
    int buffSize = atoi(ui->lineEdit_buffSize->text().toAscii());
    char* buff = new char [buffSize*1024*1024];
    int result = sky_sdfs_write(testfd,buff,(buffSize*1024*1024)*sizeof(char));
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
    delete [] buff;
    //    sky_sdfs_cleanup();
}

void MainWindow::on_test_90_2_clicked()
{
    //    sky_sdfs_init("config.ini");
    long long testfid = sky_sdfs_createfile("testfilename",BLOCKLENGTH,1);
    int testfd = sky_sdfs_openfile(testfid,O_READ);
    sky_sdfs_close(testfd);
//    char buff[BUFFSIZE];
    int buffSize = atoi(ui->lineEdit_buffSize->text().toAscii());
    char* buff = new char [buffSize*1024*1024];
    int result = sky_sdfs_read(testfd,buff,(buffSize*1024*1024)*sizeof(char));
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
    delete [] buff;
}

void MainWindow::on_readFileButton_clicked()
{
    info = new fileinfo;
    readFileID = atoi(ui->lineEdit_6->text().toAscii());
    int fd = sky_sdfs_openfile(readFileID,O_READ);
//       qDebug()<<fd;
    if(fd > 0){
        sky_sdfs_fileinfo(readFileID,info);
//        qDebug()<<info->name;
        QFileInfo fileInfo(info->name);
        QString filename = fileInfo.fileName();
        long long offset = ui->offset_Edit->text().toLongLong();
        int readSize = ui->readSize_edit->text().toInt();
        QString testFileName="";
        if(offset != 0){
            testFileName = "offset_" + QString("%1").arg(offset) + "-";
        }
        if(readSize != 0){
            testFileName = testFileName + "readSize_" + QString("%1").arg(readSize) + "-";
        }

        QFile testFile("FileId_" + QString("%1").arg(readFileID) + "-" + testFileName + filename);
        sky_sdfs_lseek(fd,offset,SEEK_SET);
        if(testFile.open(QIODevice::WriteOnly)){
            int buffSize = atoi(ui->lineEdit_buffSize->text().toAscii());
            if(buffSize > readSize){
                buffSize = readSize;
            }
            qDebug()<<"buffSize"<<buffSize;
            char* buff = new char [buffSize*1024*1024];
            while(TRUE){
                int result = 0;
                if(readSize >= buffSize){
                    result = sky_sdfs_read(fd,buff,(buffSize*1024*1024)*sizeof(char));
                    qDebug()<<"read1= "<<result;
                }
                else{
                    if(readSize <= 0){
                        ui->textEdit->append(QString::number(lineCount)+ " ----->" + testFile.fileName() + "  file download OK");
                        lineCount++;
                        break;
                    }
                    result = sky_sdfs_read(fd,buff,(readSize*1024*1024)*sizeof(char));
                    qDebug()<<"read2= "<<result;
                }
                if (result > 0){
                    qint64 writelength = testFile.write(buff,result);
                }
                else{
                    if(result == -1){
                        char name1[100];
                        qDebug()<<"ERROR:"<<getlasterror(fd,name1,100)<<name1;
                        break;
                    }
                    else{
//                        qDebug()<<result;
                        ui->textEdit->append(QString::number(lineCount)+ " ----->" + testFile.fileName() + "  file download OK");
                        lineCount++;
                        break;
                    }

                }
                readSize = readSize - buffSize;
                qDebug()<<"readSizeEND= "<<readSize;
            }
             delete [] buff;
            testFile.close();
        }
    }
    else{
        ui->textEdit->append(QString::number(lineCount)+ " ----->   FileID= " + QString::number(readFileID) + " file not find");
        lineCount++;
    }

    delete info;
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
//                        QString startTime = "2012-06-29 12:12:12.012";

        //        QDateTime qdate = QDateTime::currentDateTime();
        QString startTime = ui->dateTimeEdit->text();
                qDebug()<<startTime;
        //        QString startTime = qdate.toString("yyyy-MM-dd hh:mm:ss.zzz");

        //                qDebug()<<startTime;
        int copies = atoi(ui->lineEdit_copySize->text().toAscii());
        qDebug()<<videoFile;
        QFileInfo fileInfo1(videoFile);
        long long videoFid = sky_sdfs_createfile_ex(fileInfo1.fileName().toAscii().constData(),
                                                    BLOCKLENGTH,
                                                    copies,
                                                    NORMAL_FILE,
                                                    startTime.toAscii().data(),
                                                    0);

        if(uploadFile(videoFid,videoFile)){
            QFileInfo fileInfo2(idxFile);
            long long idxFid = sky_sdfs_createfile_ex(fileInfo2.fileName().toAscii().constData(),
                                                      BLOCKLENGTH,
                                                      copies,
                                                      INDEX_FILE,
                                                      startTime.toAscii().data(),
                                                      videoFid);
            if(uploadFile(idxFid,idxFile)){
                ui->textEdit->append("videoFid= "
                                     +  QString::number(videoFid)
                                     + "    idxFid= "
                                     + QString::number(idxFid));
            }
            else{
                ui->textEdit->append("videoFid= "
                                     +  QString::number(videoFid)
                                     + "    idx  upload fail");
            }
        }
        else{
            ui->textEdit->append("video upload fail");
        }
    }
}

void MainWindow::on_readFileInfo_clicked()
{
    info = new fileinfo;
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
    <<QString::fromUtf8(info->name)
    <<info->owner;
    if(res == 0){
        ui->textEdit->append("================ File Info ================");
        ui->textEdit->append("readFileID: " + QString::number(readFileID));
        ui->textEdit->append("beginTime: " + QString(info->beginTime));
        ui->textEdit->append("blocklength: " + QString::number(info->blocklength));
        ui->textEdit->append("blocksize: " + QString::number(info->blocksize));
        ui->textEdit->append("copysize: " + QString::number(info->copysize));
        ui->textEdit->append("fileid: " + QString::number(info->fileid));
        ui->textEdit->append("filemode: " + QString::number(info->filemode));
        ui->textEdit->append("filetype: " + QString::number(info->filetype));
        ui->textEdit->append("link: " + QString::number(info->link));
        ui->textEdit->append("name: " + QString::fromUtf8(info->name));
        ui->textEdit->append("owner: " + QString::number(info->owner));
        ui->textEdit->append("================ File Info ================");
    }
    else{
        ui->textEdit->append("======= not ================ find =========");
        ui->textEdit->append("readFileID: " + QString::number(readFileID) + "   file not find");
        ui->textEdit->append("======= not ================ find =========");
    }
    delete info;
}

void MainWindow::on_deleteFileButton_clicked()
{
    readFileID = atoi(ui->lineEdit_6->text().toAscii());
    int result = sky_sdfs_deletefile(readFileID);
    if(result == -1){
        char name1[100];
        int errcode = getlasterror(result,name1,100);
        qDebug()<<"ERROR:"<<errcode<<name1;
    }
    else{
        ui->textEdit->append("FileID: " + QString::number(readFileID) + " was DELETED");
    }
}

void MainWindow::textDown()
{
    ui->textEdit->moveCursor(QTextCursor::End);
}
