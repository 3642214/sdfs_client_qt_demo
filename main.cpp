#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QTextCodec>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    if(QString::compare(argv[1],"-m") == 0){
        w.setMode(QString(argv[2]).toInt());
    }

    if(QString::compare(argv[1],"-u") == 0){
        w.hide();
        w.upFile(QString(argv[2]).toInt(),
                 QString(argv[3]).toInt(),
                 QString(argv[4]).toInt(),
                 QString(argv[5]).toInt(),
                 QString(argv[6]).toInt());
    }

    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QTextCodec::setCodecForLocale(codec);

    QTextCodec::setCodecForCStrings(codec);

    QTextCodec::setCodecForTr(codec);
    return a.exec();
}
