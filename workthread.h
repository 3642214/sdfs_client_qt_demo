#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#define T_CRTEATE			1
#define T_OPEN			2
#define T_WRITE			3
#define T_READ			4

#include <QThread>

typedef struct testinfo{
    int        testFunc;
    QString fileName;
    int		copysize;
    int 		blocksize;
    int 		blocklength;
    long      buffsize;
    long      filesize;
    int        openMode;
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

private:
    void init();
    void testCreate();
    void testOpen(int mode);
    void testWrite();
    void testRead();
    void close();

private:
    long long fileID;
    int fd;
    testinfo* testinfo1;
    int errorTotal;
    int tureTotal;

public:
    QString name;
};

#endif // WORKTHREAD_H
