#include "workthread.h"
#include "SkySDFSSDK.h"
#include "QDebug"
#include "QFile"
#include "QFileInfo"

#define BUFFSIZE		1024*1024
#define BLOCKLENGTH     256*1024*1024

workThread::workThread(QObject *parent) :
    QThread(parent)
{
}

workThread::~workThread()
{
    quit();
    wait();
}

void workThread::run()
{
    switch(testinfo1->testFunc)
    {
    case T_CRTEATE:
    {
        qDebug()<<this->name<<"T_CRTEATE thread start";
        this->testCreate();
        break;
    }
    case T_OPEN:
    {
        qDebug()<<this->name<<"T_OPEN mode"<<testinfo1->openMode<<"thread start";
        this->testOpen(testinfo1->openMode);
        break;
    }
    case T_WRITE:
    {
        qDebug()<<this->name<<"T_WRITE thread start";
        this->testWrite();
        break;
    }
    case T_READ:
    {
        qDebug()<<this->name<<"T_READ thread start";
        this->testRead();
        break;
    }
    case T_UPLOAD:
    {
        qDebug()<<this->name<<"T_UPLOAD thread start";
        this->testUpload();
        break;
    }
    }
}

void workThread::testCreate()
{
    this->init();
    fileID = sky_sdfs_createfile(name.toAscii().constData(),testinfo1->blocklength*1024*1024,testinfo1->copysize);
    qDebug()<<"Thread"<<name<<":"<<"fileID="<<fileID<<";blocklength="<<testinfo1->blocklength<<";copysize="<<testinfo1->copysize;
    if(fileID > 0){
        emit changeText(name + "    testCreate   true");
        //        testinfo1->result = name + "    testCreate    true";
        //        qDebug()<<"Thread "<<name<<"Create ------------------------->succ";
    }
    else{
        char errname[100];
        emit changeText(name
                        + "    testCreate   false  ErrorNO = "
                        + QString::number(getlasterror(-1,errname,100))
                        + "   "
                        + errname);
        //        testinfo1->result = name + "testCreate   false";
        this->quit();
        //        qDebug()<<"Thread"<<name<<":"<<"Create ------------------------->fail";
    }

}

void workThread::testOpen(int mode)
{
    this->testCreate();
    fd = sky_sdfs_openfile(fileID,mode);
    if(fileID > 0){
        emit changeText(name + "    testOpen   true  fd= " + QString::number(fd));
        //        testinfo1->result = name + "    testOpen   true";
        //        qDebug()<<"Thread"<<name<<":"<<"fd="<<fd;
        //        qDebug()<<"Thread "<<name<<"Open ------------------------->succ";
    }
    else{
        char errname[100];
        emit changeText(name
                        + "    testOpen   false  ErrorNO = "
                        + QString::number(getlasterror(-1,errname,100))
                        + "   "
                        + errname);
        //        testinfo1->result = name + "    testOpen   false";
        //        qDebug()<<"Thread"<<name<<":"<<"Open ------------------------->fail";
        this->quit();
    }


}

void workThread::testWrite()
{
    this->testOpen(O_WRITE);
    char* buffer = new char [testinfo1->buffsize*1024*1024];
    qDebug()<<"Thread"<<name<<":"<<"buffsize="<<testinfo1->buffsize;
    int count = testinfo1->filesize  / testinfo1->buffsize;
    for(int i = 1;i<=count;i++)
    {
        //        qDebug()<<"Thread"<<name<<":"<<"write start";
        int result = sky_sdfs_write(fd,buffer,(testinfo1->buffsize*1024*1024)*sizeof(char));
        qDebug()<<"Thread"<<name<<":"<<result<<i<<"/"<<count<<"write stop";
        if(result == -1)
        {
            char errname[100];
            qDebug()<<getlasterror(fd,errname,100)<<errname;
            qDebug()<<"upload retry";
            result = sky_sdfs_write(fd,buffer,(testinfo1->buffsize*1024*1024)*sizeof(char));
            if(result == -1){
                char errname[100];
                emit changeText(name
                                + "    testWrite   false  ErrorNO = "
                                + QString::number(getlasterror(fd,errname,100))
                                + "   "
                                + errname);
                //                testinfo1->result = name + "    testWrite   false";
                qDebug()<<"Thread "<<name<<"upload fail";
                qDebug()<<"Thread"<<name<<":"<<"Write ------------------------->fail";
                delete [] buffer;
                return;
            }
        }
    }
    emit changeText(name + "    testWrite   true  fileID = " + QString::number(fileID));
    //    testinfo1->result = name + "      testWrite   ture";
    qDebug()<<"Thread"<<name<<":"<<"Write ------------------------->succ";
    delete [] buffer;
}

void workThread::testRead()
{
    this->testOpen(O_WRITE);
    QFile file("testReadFile");
    allUploadFileSize = 0;
    if(file.open(QIODevice::ReadOnly)){
        char buff[200*1024];
        while(!file.atEnd()){
            int size = file.read(buff,sizeof(buff));
            //                       qDebug()<<"write start"<<size;
            int result = sky_sdfs_write(fd,buff,size);
            if(result == -1){
                char errname[100];
                qDebug()<<"Thread "<<name<<"ERROR:"<<getlasterror(fd,errname,100);
                qDebug()<<"upload retry";
                result = sky_sdfs_write(fd,buff,size);
                if(result == -1){
                    qDebug()<<"Thread "<<name<<"upload fail";
                    return;
                }
                else
                    allUploadFileSize += result;
            }
            else{
                allUploadFileSize += result;
            }
        }
    }
    qDebug()<<"Thread "<<name<<"allUploadFileSize"<<allUploadFileSize/1024/1024;
    file.close();
    close();
    int fd = sky_sdfs_openfile(fileID,O_READ);
    //   qDebug()<<fd;
    allDownloadFileSize = 0;
    QFile testFile("testFile" + name);
    if(testFile.open(QIODevice::WriteOnly)){
        char buff[200*1024];
        while(TRUE){
            int result = 0;
            result = sky_sdfs_read(fd,buff,sizeof(buff));
            //                      qDebug()<<"Thread "<<name<<"read= "<<result;
            if (result > 0){
                qint64 writelength = testFile.write(buff,result);
                //                                qDebug()<<"Thread "<<name<<writelength<<testFile.size()/1024/1024;
                allDownloadFileSize += writelength;
            }
            else{
                if(result == -1){
                    char name1[100];
                    qDebug()<<"Thread "<<name<<"ERROR:"<<getlasterror(fd,name1,100);
                }
                else{
                    qDebug()<<"Thread "<<name<<"allDownloadFileSize"<<allDownloadFileSize/1024/1024;
                    break;
                }

            }
        }
        testFile.close();
    }
    if(allDownloadFileSize == allUploadFileSize){
        emit changeText(name + "    testRead   true");
        //        testinfo1->result = name + "    testRead   true";
        qDebug()<<"Thread "<<name<<"Read ------------------------->succ";
    }
    else{
        emit changeText(name + "    testRead   false");
        //        testinfo1->result = name + "    testRead   false";
        qDebug()<<"Thread "<<name<<"Read ------------------------->fail";
    }
    //   if(checkFile(file,testFile)){

    //   }
}

void workThread::testUpload()
{
    for(int i = 1;i<=testinfo1->count;i++){
        for(int j = 0;j<testinfo1->filePath.length();j++){
            QFile file(testinfo1->filePath.at(j));
            QFileInfo fileInfo(file);
            long long fileFid = sky_sdfs_createfile(fileInfo.fileName().toUtf8().constData(),
                                                    testinfo1->blocklength*1024*1024,
                                                    testinfo1->copysize);
            if(fileFid >0){
                if(uploadFile(fileFid,testinfo1->filePath.at(j))){
                    emit changeText("count ="
                                    + QString::number(i)
                                    + "  upload ok . fileID = "
                                    + QString::number(fileFid)
                                    + "  fileName = "
                                    +fileInfo.fileName());
                    qDebug()<<"count = "<<i<<" filelist.at="<<j<<fileInfo.fileName()<<"  upload ok";
                }
                else{
                    emit changeText("count ="
                                    + QString::number(i)
                                    + "  upload fail: "
                                    + errorCode
                                    + "  fileID = "
                                    + QString::number(fileFid));
                    qDebug()<<"count = "<<i<<" filelist.at="<<j<<fileInfo.fileName()<<"  upload fail";
                }
            }
            else{
                qDebug()<<"Thread "<<name<<"ERROR:"<<getlasterror(-1,errorCode,100)<<errorCode;
                emit changeText("count ="
                                + QString::number(i)
                                + "  upload fail(create): "
                                + errorCode);
            }
        }
    }
}

bool workThread::uploadFile(long long fileFid, QString fileName)
{

    bool res = false;
    QFile file(fileName);
    int fileFd = sky_sdfs_openfile(fileFid,O_WRITE);
    if(file.open(QIODevice::ReadOnly) and fileFid > 0 and fileFd > 0){
        char buff[BUFFSIZE];
        while(!file.atEnd()){
            int size = file.read(buff,sizeof(buff));
            int result = sky_sdfs_write(fileFd,buff,size);
            if(result == -1){
                qDebug()<<"Thread "<<name<<"ERROR:"<<getlasterror(fileFd,errorCode,100)<<errorCode;
                break;
            }
            if(file.atEnd() and result != -1){
                res = true;
            }
        }
        file.close();
//        sky_sdfs_close(fileFd);
    }
//    while(true){
    qDebug()<<res;
    return res;
//    }
}


void workThread::close()
{
    sky_sdfs_close(fd);
    qDebug()<<"Thread "<<name<<"fd= "<<fd<<" closed";
}

bool workThread::checkFile(QFile file1, QFile file2)
{

}

void workThread::init()
{
    //    sky_sdfs_init("config.ini");
}
