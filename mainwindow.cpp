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
    connect(ui->textEdit,SIGNAL(textChanged()),this,SLOT(textDown()));
    connect(ui->checkBox,SIGNAL(clicked()),this,SLOT(changeValue()));
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
//            qDebug()<<1;
        }
//        qDebug()<<threadList->at(0)->isRunning()<<"2";
    }
    delete ui;
}

void MainWindow::setMode(int Mode)
{
    if(Mode == ADVANCE_MODE){
        ui->groupBox_AutoTest->setEnabled(true);
        ui->groupBox_TestLink->setEnabled(true);
        ui->deleteFileButton->setEnabled(true);
    }
    else
    {
        ui->groupBox_AutoTest->setEnabled(false);
        ui->groupBox_TestLink->setEnabled(false);
        ui->deleteFileButton->setEnabled(false);
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
    test->downloadAll = ui->checkBox->isChecked();
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
//        connect(thread,SIGNAL(finished()),this,SLOT(threadOver()));
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

void MainWindow::changeValue()
{
    ui->readSize_edit->setEnabled(!ui->readSize_edit->isEnabled());
}

bool MainWindow::uploadFile(long long fileFid,QString fileName)
{
    bool res = false;
    long long allrst = 0;
    int fileFd = sky_sdfs_openfile(fileFid,O_WRITE);
    qDebug()<<"fileName= "<<fileName<<" fileFid="<<fileFid<<" fileFd="<<fileFd;
    QFile file(fileName);
    qDebug()<<"file.size"<<file.size();
    if(file.open(QIODevice::ReadOnly) and fileFid > 0 and fileFd > 0 and file.size() > 0){
//        char buff[BUFFSIZE];
        int buffSize = atoi(ui->lineEdit_buffSize->text().toAscii());
        char* buff = new char [buffSize*1024*1024];
        while(!file.atEnd()){
            int size = file.read(buff,(buffSize*1024*1024)*sizeof(char));
                      qDebug()<<"read file size = "<<size	;
            qDebug()<<"write start";
            int result = sky_sdfs_write(fileFd,buff,size);
            qDebug()<<"result"<<result;
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
        delete [] buff;
    }
    else{
        qDebug()<<fileName<<" open failed!!! or Fid|Fd < 0 or file.size = 0";
    }
    file.close();
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
//                disconnect(threadList->at(i),SIGNAL(changeText(QString)),this,SLOT(setLog(QString)));
                threadList->at(i)->deleteLater();
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
    changeTestinfo();
    test->fileID = ui->lineEdit_6->text().toLongLong();
    if(ui->lineEdit_6->text().length() >= 19){
        test->testFunc = T_DOWNLOAD_LFILE;
    }
    else{
        test->downloadSize = atoi(ui->readSize_edit->text().toAscii());
        test->offset = atoi(ui->offset_Edit->text().toAscii());
        test->testFunc = T_DOWNLOAD;
    }
    createThread();
    runThread();
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
        changeTestinfo();
        test->fileName = videoFile;
        test->idxName = idxFile;
        test->startTime = ui->dateTimeEdit->text();
        test->count = ui->uploadCount->text().toInt();
        test->testFunc = T_UPLOAD_EX;
        createThread();
        runThread();
    }
}

void MainWindow::on_readFileInfo_clicked()
{
    info = new fileinfo;
    readFileID = ui->lineEdit_6->text().toLongLong();
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
    readFileID = ui->lineEdit_6->text().toLongLong();
    int result = sky_sdfs_deletefile(readFileID);
    if(result == -1){
        char name1[100];
        int errcode = getlasterror(result,name1,100);
        ui->textEdit->append("DELETE FileID: " + QString::number(readFileID) + " ERRORCODE:" + QString::number(errcode) + " " +name1);
//        qDebug()<<"ERROR:"<<errcode<<name1;
    }
    else{
        ui->textEdit->append("DELETE FileID: " + QString::number(readFileID) + " OK");
    }
}

void MainWindow::textDown()
{
    ui->textEdit->moveCursor(QTextCursor::End);
}

void MainWindow::on_lockFileButton_clicked()
{
    readFileID = ui->lineEdit_6->text().toLongLong();
    int result = sky_sdfs_lockfile(readFileID);
    if(result == -1){
        char name1[100];
        int errcode = getlasterror(result,name1,100);
        ui->textEdit->append("LOCK FileID: " + QString::number(readFileID) + " ERRORCODE:" + QString::number(errcode) +" " +name1);
//        qDebug()<<"ERROR:"<<errcode<<name1;
    }
    else{
        ui->textEdit->append("LOCK FileID: " + QString::number(readFileID) + " OK ");
    }
}

void MainWindow::on_unLockFileButton_clicked()
{
    readFileID = ui->lineEdit_6->text().toLongLong();
    int result = sky_sdfs_unlockfile(readFileID);
    if(result == -1){
        char name1[100];
        int errcode = getlasterror(result,name1,100);
        ui->textEdit->append("UNLOCK FileID: " + QString::number(readFileID) + " ERRORCODE:" + QString::number(errcode) +" " +name1);
//        qDebug()<<"ERROR:"<<errcode<<name1;
    }
    else{
        ui->textEdit->append("UNLOCK FileID: " + QString::number(readFileID) + " OK ");
    }
}

void MainWindow::on_serachButton_clicked()
{
    QString time = ui->dateTimeEdit->text();
    int fileID = atoi(ui->lineEdit_6->text().toAscii());
    int fd = sky_sdfs_openfile(fileID,O_READ);
    int result = sky_sdfs_search(fd,time.toLatin1().data(),STARTTIME);
//    qDebug()<<result;
    if(result == -1){
        char name1[100];
        int errcode = getlasterror(result,name1,100);
        ui->textEdit->append("TimeToOffset: " + QString::number(fileID) + " ERRORCODE:" + QString::number(errcode) +" " +name1);
    }
    else{
        ui->textEdit->append("Time:" + time + " To Offset:" + QString::number(result));
    }
}

void MainWindow::on_upLittleFileButton_clicked()
{
    QList<QString> fileList = QFileDialog::getOpenFileNames(
                this,
                QDir::currentPath());

    if (!fileList.isEmpty()) {
        changeTestinfo();
        ui->selectFileCount->setText(QString::number(fileList.length()));
        test->filePath = fileList;
        test->count = ui->uploadCount->text().toInt();
        test->testFunc = T_UPLOAD_LFILE;
        createThread();
        runThread();
    }
}

void MainWindow::on_pushButton_clicked()
{
    sky_sdfs_close_pipe();
//    sky_sdfs_cleanup();
}
