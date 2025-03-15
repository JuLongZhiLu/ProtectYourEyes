#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false); // 阻止因窗口关闭退出程序
    MainWindow w;
    return a.exec();
}
