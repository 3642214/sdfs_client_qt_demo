#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#define T_CRTEATE		1
#define T_OPEN			2
#define T_WRITE			3
#define T_READ			4
#define T_UPLOAD        5
#define T_UPLOAD_EX     6
#define T_DOWNLOAD      7
#define T_UPLOAD_LFILE  8
#define T_DOWNLOAD_LFILE 9
#define T_UPLOAD_FILE 10
#define T_DOWNLOAD_NEW 11

#include <QThread>
#include <QFile>
#include <QTime>
#include "SkySDFSSDK.h"

typedef struct testinfo{
    int     testFunc;
    QString fileName;
    QString idxName;
    QString startTime;
    long long fileID;
    int     offset;
    int     downloadSize;
    bool    downloadAll;
    int		copysize;
    int 	blocksize;
    int 	blocklength;
    long    buffsize;
    long    filesize;
    int     openMode;
    QString result;
    QList<QString>  filePath;
    int       count;
    }testinfo;

class workThread : public QThread
{
    Q_OBJECT
public:
    explicit workThread(QObject *parent = 0);
    ~workThread();
    workThread(QString name,testinfo* test){
    this->name = name;
    this->testinfo1 = test;
    }
    void run();

public slots:
    fileinfo* setFileInfo(QString fileName,int blockLength,int copySize);

signals:
    void changeText(QString str);
private:
    void init();
    void testCreate();
    void testOpen(int mode);
    void testWrite();
    void testRead();
    void testUpload();
    void testUpload_ex();
    void testDownload();
    void testUploadLFile();
    void testDownloadLFile();
    void testUploadFile();
    void testNewDownload();
    bool uploadFile(long long fileFid, QString fileName);
    void close();

private:
    long long fileID;
    int fd;
    testinfo* testinfo1;
    int errorTotal;
    int tureTotal;
    bool checkFile(QFile file1,QFile file2);
    qint64 allDownloadFileSize;
    qint64 allUploadFileSize ;
    char errorCode[100];


public:
    QString name;
    QTime time;


};

#endif // WORKTHREAD_H
