#include "workthread.h"
#include "SkySDFSSDK.h"
#include "QDebug"
#include "QFile"
#include "QFileInfo"
#include "mainwindow.h"

#define BUFFSIZE		1024*1024
#define BLOCKLENGTH     256*1024*1024

workThread::workThread(QObject *parent) :
    QThread(parent)
{
}

workThread::~workThread()
{
    this->close();
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
    case T_UPLOAD_EX:
    {
        qDebug()<<this->name<<"T_UPLOAD_EX thread start";
        this->testUpload_ex();
        break;
    }
    case T_DOWNLOAD:
    {
        qDebug()<<this->name<<"T_DOWNLOAD thread start";
        this->testDownload();
        break;
    }
    case T_UPLOAD_LFILE:
    {
        qDebug()<<this->name<<"T_UPLOAD_LFILE thread start";
        this->testUploadLFile();
        break;
    }
    case T_UPLOAD_FILE:
    {
        qDebug()<<this->name<<"T_UPLOAD_FILE thread start";
        this->testUploadFile();
        break;
    }
    case T_DOWNLOAD_LFILE:
    {
        qDebug()<<this->name<<"T_DOWNLOAD_LFILE thread start";
        this->testDownloadLFile();
        break;
    }
    case T_DOWNLOAD_NEW:
    {
        qDebug()<<this->name<<"T_DOWNLOAD_NEW thread start";
        this->testNewDownload();
        break;
    }
    }
    //    this->close();
    //    client_close(fd);
    //    qDebug()<<"Thread "<<name<<"fd= "<<fd<<" closed";
}

void workThread::testCreate()
{
    this->init();
    time.start();

    struct fileinfo finfo = {
        finfo.copysize=testinfo1->copysize,
        finfo.filetype=NORMAL_FILE,
        finfo.blocklength=testinfo1->blocklength*1024*1024,
    };
    memcpy(finfo.name,name.toAscii().data(),100);

    fileID = client_create(&finfo);

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
    fd = client_open(fileID,mode);
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
        int result = client_write(fd,buffer,(testinfo1->buffsize*1024*1024)*sizeof(char));
        qDebug()<<"Thread"<<name<<":"<<result<<i<<"/"<<count<<"write stop";
        if(result == -1)
        {
            char errname[100];
            qDebug()<<getlasterror(fd,errname,100)<<errname;
            qDebug()<<"upload retry";
            result = client_write(fd,buffer,(testinfo1->buffsize*1024*1024)*sizeof(char));
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
    int timeDiff = time.elapsed();
    float uploadSpeed = testinfo1->filesize / ( timeDiff / 1000.0 );
    emit changeText("used time :" + QString::number(timeDiff) + " ms , upload speed :" + QString::number(uploadSpeed) + " M/s");
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
            int result = client_write(fd,buff,size);
            if(result == -1){
                char errname[100];
                qDebug()<<"Thread "<<name<<"ERROR:"<<getlasterror(fd,errname,100);
                qDebug()<<"upload retry";
                result = client_write(fd,buff,size);
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
    int fd = client_open(fileID,O_READ);
    //   qDebug()<<fd;
    allDownloadFileSize = 0;
    QFile testFile("testFile" + name);
    if(testFile.open(QIODevice::WriteOnly)){
        char buff[200*1024];
        while(TRUE){
            int result = 0;
            result = client_read(fd,buff,sizeof(buff));
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
            fileinfo* finfo =setFileInfo(fileInfo.fileName().toUtf8().constData(),testinfo1->blocklength*1024*1024,testinfo1->copysize);
            finfo->filetype = NORMAL_FILE;
            long long fileFid = client_create(finfo);
            if(fileFid >0){
                if(uploadFile(fileFid,testinfo1->filePath.at(j))){
                    emit changeText("Count ="
                                    + QString::number(i)
                                    + " Thread ="
                                    + name
                                    + "  Upload ok . FileID = "
                                    + QString::number(fileFid)
                                    + "  FileName = "
                                    +fileInfo.fileName());
                    qDebug()<<"count = "<<i<<" filelist.at="<<j<<fileInfo.fileName()<<"  upload ok";
                }
                else{
                    emit changeText("Count ="
                                    + QString::number(i)
                                    + " Thread ="
                                    + name
                                    + "  Upload Fail: "
                                    + errorCode
                                    + "  FileID = "
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

void workThread::testUpload_ex()
{
    for(int i = 1;i<=testinfo1->count;i++){
        QFileInfo fileInfo(testinfo1->fileName);
        QFileInfo idxFileInfo(testinfo1->idxName);

        fileinfo* finfo =setFileInfo(fileInfo.fileName().toUtf8().constData(),testinfo1->blocklength*1024*1024,testinfo1->copysize);
        memcpy(finfo->beginTime,testinfo1->startTime.toAscii().data(),24);
        finfo->filetype = NORMAL_FILE;
        long long fileFid = client_create(finfo);

        memcpy(finfo->name,idxFileInfo.fileName().toAscii().constData(),100);
        finfo->filetype = INDEX_FILE;
        finfo->link = fileFid;
        long long idxFid = client_create(finfo);

        if(fileFid > 0 and idxFid > 0){
            if(uploadFile(fileFid,testinfo1->fileName) and uploadFile(idxFid,testinfo1->idxName)){
                emit changeText("Count ="
                                + QString::number(i)
                                + " Thread ="
                                + name
                                + "  Upload ok . videoFid= "
                                + QString::number(fileFid)
                                + "  idxFid = "
                                + QString::number(idxFid));
                qDebug()<<"count = "<<i<<"video:"<<fileInfo.fileName()<<"idx:"<<idxFileInfo.fileName()<<"  upload ok";
            }
            else{
                emit changeText("Count ="
                                + QString::number(i)
                                + " Thread ="
                                + name
                                + "  Upload Fail: "
                                + errorCode
                                + "  FileID = "
                                + QString::number(fileFid));
                qDebug()<<"count = "<<i<<fileInfo.fileName()<<"idx:"<<idxFileInfo.fileName()<<"  upload fail";
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

void workThread::testDownload()
{
    fileinfo* info = new fileinfo;
    long long readFileID = testinfo1->fileID;
    fd = client_open(readFileID,O_READ);
    //       qDebug()<<fd;
    if(fd > 0){
        get_fileinfo(readFileID,info);
        //        qDebug()<<info->name;
        QFileInfo fileInfo(info->name);
        QString filename = fileInfo.fileName();
        long long offset = testinfo1->offset;
        int downloadSize = testinfo1->downloadSize;
        int blockLenth = testinfo1->blocklength;
        int buffSize = testinfo1->buffsize;
        QString testFileName="";
        if(offset != 0){
            int seqNo = offset/(blockLenth*1024*1024);
            if(offset%(blockLenth*1024*1024)>0){
                seqNo++;
            }
            testFileName = "seqNo_" + QString("%1").arg(seqNo) + "-";
        }
        if(downloadSize != 0 and !testinfo1->downloadAll){
            testFileName = testFileName + "downloadSize_" + QString("%1").arg(downloadSize) + "M-";
        }

        QString FName = "Thread_" + name + "_FId_" + QString("%1").arg(readFileID) + "-" + testFileName + filename;
        QFile testFile( FName + ".tmp");
        client_postion(fd,offset,SEEK_SET);
        if(testFile.open(QIODevice::WriteOnly)){
            if(buffSize > downloadSize){
                buffSize = downloadSize;
            }
            //            qDebug()<<"buffSize"<<buffSize;
            char* buff = new char [buffSize*1024*1024];
            while(TRUE){
                int result = 0;
                qint64 writelength = 0;
                result = client_read(fd,buff,(downloadSize*1024*1024)*sizeof(char));
                //                    qDebug()<<"read2= "<<result;
                if (result > 0){
                    writelength += testFile.write(buff,result);
                    if(result <= (downloadSize*1024*1024)*sizeof(char)){
                        emit changeText("Thread ="
                                        + name
                                        + "  Download ok. fileName: "
                                        + FName);
                        testFile.rename(FName);
                        break;
                    }
                }
                else if(result == -1){
                    char name1[100];
                    int errorCode = getlasterror(fd,name1,100);
                    qDebug()<<"ERROR:"<<errorCode<<name1;
                    emit changeText("Thread ="
                                    + name
                                    + " FileID = "
                                    + QString::number(readFileID)
                                    + " ERROR:"
                                    + QString::number(errorCode)
                                    + name1
                                    );
                    testFile.remove();
                    break;
                }
                else if(result == 0){
                    if(writelength > 0){
                        emit changeText("Thread ="
                                        + name
                                        + "  Download ok. fileName: "
                                        + FName);
                        testFile.rename(FName);
                        break;
                    }
                    else{
                        emit changeText("Thread ="
                                        + name
                                        + " No Data To Download(EOF).");
                        qDebug()<<testFile.fileName();
                        testFile.remove();
                        break;
                    }

                }
                if(!testinfo1->downloadAll){
                    downloadSize = downloadSize - buffSize;
                    //                qDebug()<<"downloadSizeEND= "<<downloadSize;
                }
            }
            delete [] buff;
            testFile.close();
        }
    }
    else{
        emit changeText("Thread ="
                        + name
                        + "  Download FAIL. FileID= "
                        + QString::number(readFileID)
                        + " file not find");
    }

    delete info;
}

void workThread::testUploadLFile()
{
    for(int i = 1;i<=testinfo1->count;i++){
        for(int j = 0;j<testinfo1->filePath.length();j++){
            QFile file(testinfo1->filePath.at(j));
            QFileInfo fileInfo(file);
            //            long long fileFid = 0;
            fileinfo* finfo =setFileInfo(fileInfo.fileName().toUtf8().constData(),testinfo1->blocklength*1024*1024,testinfo1->copysize);
            //            memcpy(finfo->beginTime,testinfo1->startTime.toAscii().data(),24);
            finfo->filetype = LITTLE_FILE;
            finfo->fileid = 0;
            long long fileFid = client_upload(finfo,testinfo1->filePath.at(j).toUtf8().constData());
            qDebug()<<fileFid<<testinfo1->filePath.at(j).toUtf8().constData()<<finfo->beginTime;
            if(fileFid >0){
                //                if(uploadFile(fileFid,testinfo1->filePath.at(j))){
                emit changeText("Count ="
                                + QString::number(i)
                                + " Thread ="
                                + name
                                + "  Upload ok . FileID = "
                                + QString::number(fileFid)
                                + "  FileName = "
                                +fileInfo.fileName());
                qDebug()<<"count = "<<i<<" filelist.at="<<j<<fileInfo.fileName()<<"  upload ok";
                //                }
                //                else{
                //                    emit changeText("Count ="
                //                                    + QString::number(i)
                //                                    + " Thread ="
                //                                    + name
                //                                    + "  Upload Fail: "
                //                                    + errorCode
                //                                    + "  FileID = "
                //                                    + QString::number(fileFid));
                //                    qDebug()<<"count = "<<i<<" filelist.at="<<j<<fileInfo.fileName()<<"  upload fail";
                //                }
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

void workThread::testUploadFile()
{
    for(int i = 1;i<=testinfo1->count;i++){
        for(int j = 0;j<testinfo1->filePath.length();j++){
            QFile file(testinfo1->filePath.at(j));
            QFileInfo fileInfo(file);
            fileinfo* finfo =setFileInfo(fileInfo.fileName().toUtf8().constData(),testinfo1->blocklength*1024*1024,testinfo1->copysize);
            memcpy(finfo->beginTime,testinfo1->startTime.toAscii().data(),24);
            finfo->filetype = NORMAL_FILE;
            finfo->fileid = 0;
            long long fileFid = client_upload(finfo,testinfo1->filePath.at(j).toUtf8().constData());
            qDebug()<<fileFid<<testinfo1->filePath.at(j).toUtf8().constData()<<finfo->beginTime;
            if(fileFid >0){
                //                if(uploadFile(fileFid,testinfo1->filePath.at(j))){
                emit changeText("Count ="
                                + QString::number(i)
                                + " Thread ="
                                + name
                                + "  Upload ok . FileID = "
                                + QString::number(fileFid)
                                + "  FileName = "
                                +fileInfo.fileName());
                qDebug()<<"count = "<<i<<" filelist.at="<<j<<fileInfo.fileName()<<"  upload ok";
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

void workThread::testDownloadLFile()
{
    long long readFileID = testinfo1->fileID;
    for(int i = 1;i<=testinfo1->count;i++){
        QFile testFile("Thread_" + name + "count_" + QString("%1").arg(i) + "_FId_" + QString("%1").arg(readFileID) + "-" + "littleFile.tmp");
        if(client_download(readFileID,testFile.fileName().toAscii().data()) > 0){
            emit changeText("Thread ="
                            + name
                            + " Download ok. fileName: "
                            + testFile.fileName());
            testFile.rename("Thread_" + name + "count_" + QString("%1").arg(i) + "_FId_" + QString("%1").arg(readFileID) + "-" + "littleFile");
        }
        else{
            char name1[100];
            int errorCode = getlasterror(fd,name1,100);
            qDebug()<<"ERROR:"<<errorCode<<name1;
            emit changeText("Thread ="
                            + name
                            + " FileID = "
                            + QString::number(readFileID)
                            + " ERROR:"
                            + QString::number(errorCode)
                            + name1);
            testFile.close();
            QFile::remove(testFile.fileName());
        }
    }
}

bool workThread::uploadFile(long long fileFid, QString fileName)
{

    bool res = false;
    QFile file(fileName);

    switch(testinfo1->testFunc)
    {
    case T_UPLOAD_LFILE:
    {
        //        fd = client_open(fileFid);

        break;
    }
    default:
    {
        fd = client_open(fileFid,O_WRITE);
    }
    }
    qDebug()<<"~~~~~~id= "<<fileFid<<"  FD= "<<fd;

    if(file.open(QIODevice::ReadOnly) and fileFid > 0 and fd > 0){
        //        char buff[BUFFSIZE];
        char* buff = new char [testinfo1->buffsize*1024*1024];
        while(!file.atEnd()){
            int size = file.read(buff,(testinfo1->buffsize*1024*1024)*sizeof(char));
            int result;
            switch(testinfo1->testFunc)
            {
            case T_UPLOAD_LFILE:
            {
                //                result = client_write_littlefile(fd,buff,size);
                break;
            }
            default:
            {
                result = client_write(fd,buff,size);
            }
            }
            qDebug()<<"~~~~~~~~write result= "<<result;
            if(result == -1){
                qDebug()<<"Thread "<<name<<"ERROR:"<<getlasterror(fd,errorCode,100)<<errorCode;
                break;
            }
            if(file.atEnd() and result != -1){
                res = true;
            }
        }
        file.close();
        client_close(fd);
        delete [] buff;
    }
    //    qDebug()<<res;
    return res;
}


void workThread::close()
{
    client_close(fd);
    qDebug()<<"Thread "<<name<<"fd= "<<fd<<" closed";
}

bool workThread::checkFile(QFile file1, QFile file2)
{

}

void workThread::init()
{
    //        sky_sdfs_init("config.ini");
}


fileinfo * workThread::setFileInfo(QString fileName, int blockLength, int copySize)
{
    struct fileinfo finfo = {
        finfo.copysize=copySize,
        finfo.filetype=NORMAL_FILE,
        finfo.blocklength=blockLength*1024*1024,
    };

    memcpy(finfo.name,fileName.toAscii().constData(),100);
    return &finfo;
}

void workThread::testNewDownload()
{
    long long readFileID = testinfo1->fileID;
        QFile testFile("Thread_" + name + "_FId_" + QString("%1").arg(readFileID) + "-" + "newDownloadFile.tmp");
        if(client_download(readFileID,testFile.fileName().toAscii().data()) >= 0){
            emit changeText("Thread ="
                            + name
                            + " Download ok. fileName: "
                            + testFile.fileName());
            testFile.rename("Thread_" + name + "_FId_" + QString("%1").arg(readFileID) + "-" + "newDownloadFile");
        }
        else{
            char name1[100];
            int errorCode = getlasterror(fd,name1,100);
            qDebug()<<"ERROR:"<<errorCode<<name1;
            emit changeText("Thread ="
                            + name
                            + " FileID = "
                            + QString::number(readFileID)
                            + " ERROR:"
                            + QString::number(errorCode)
                            + name1);
            testFile.close();
            QFile::remove(testFile.fileName());
        }
}
