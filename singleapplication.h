#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>

class SingleApplication : public QApplication
{
    Q_OBJECT
public:
    SingleApplication(int &argc,char **argv);
    bool isRunning();   //检查是否已有实例运行
private slots:
    void handleNewConnection(); //  处理新连接
private:
    QLocalServer* server;
    bool m_isRunning; // 标记是否已有实例
    QString m_serverName; // 唯一服务名称

};

#endif // SINGLEAPPLICATION_H
