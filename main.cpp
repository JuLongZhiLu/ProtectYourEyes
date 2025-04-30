#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false); // 阻止因窗口关闭退出程序
    
    // load default translation
//    QTranslator translator;
//    if(translator.load(":/res/language/en_US.qm")) {
//        a.installTranslator(&translator);
//    }
    
    MainWindow w;
    return a.exec();
}
