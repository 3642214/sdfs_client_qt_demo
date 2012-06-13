#include "workthread.h"
#include "SkySDFSSDK.h"
#include "QDebug"

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
    }
}

void workThread::testCreate()
{
    this->init();
    fileID = sky_sdfs_createfile("test",testinfo1->blocklength*1024*1024,testinfo1->copysize);
    qDebug()<<"Thread"<<name<<":"<<"fileID="<<fileID<<";blocklength="<<testinfo1->blocklength<<";copysize="<<testinfo1->copysize;
    if(fileID==-1)
    {
        this->quit();
        qDebug()<<"Thread"<<name<<":"<<"Create error";
    }
}

void workThread::testOpen(int mode)
{
    this->testCreate();
    fd = sky_sdfs_openfile(fileID,mode);
    qDebug()<<"Thread"<<name<<":"<<"fd="<<fd;
}

void workThread::testWrite()
{
    this->testOpen(O_WRITE);
    char* buffer = new char [testinfo1->buffsize*1024*1024];
    qDebug()<<"Thread"<<name<<":"<<"buffsize="<<testinfo1->buffsize;
    int count = testinfo1->filesize  / testinfo1->buffsize;
    for(int i = 1;i<=count;i++)
    {
        qDebug()<<"Thread"<<name<<":"<<"write start";
        int result = sky_sdfs_write(fd,buffer,(testinfo1->buffsize*1024*1024)*sizeof(char));
        qDebug()<<"Thread"<<name<<":"<<result<<i<<"/"<<count<<"write stop";
        if(result == -1)
        {
            char name[100];
            qDebug()<<getlasterror(fd,name,100);
        }
    }
    delete [] buffer;
}

void workThread::testRead()
{
    this->testOpen(O_READ);

}

void workThread::close()
{
    sky_sdfs_close(fd);
    qDebug()<<fd<<" closed";
}

void workThread::init()
{
    sky_sdfs_init("config.ini");
}
