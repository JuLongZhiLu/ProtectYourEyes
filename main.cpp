#include "mainwindow.h"
#include "singleapplication.h"
#include <QApplication>
#include <QTranslator>
#include <QDebug>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    SingleApplication a(argc, argv);

    if(a.isRunning()){
        QMessageBox::information(nullptr,"Info","已经有程序运行了");
        return 0;
    }

    a.setQuitOnLastWindowClosed(false); // 阻止因窗口关闭退出程序
    
    // load default translation
//    QTranslator translator;
//    if(translator.load(":/res/language/en_US.qm")) {
//        a.installTranslator(&translator);
//    }
    
    MainWindow w;
    return a.exec();
}
