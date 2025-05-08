#include "singleapplication.h"


SingleApplication::SingleApplication(int &argc, char **argv)
    :QApplication(argc,argv),m_isRunning(false)
{
    //设置唯一服务名
    m_serverName = QCoreApplication::applicationName();

    //尝试连接到已有实例
    QLocalSocket socket;
    socket.connectToServer(m_serverName);
    if(socket.waitForConnected(500)){  //500ms超时
        m_isRunning = true;
        return; //退出构造函数，后续由isRunning()判断
    }

    //创建本地服务器
    server = new QLocalServer(this);
    connect(server,&QLocalServer::newConnection,this,&SingleApplication::handleNewConnection);

    //清理可能的残留（程序崩溃时未释放的socket）
    QLocalServer::removeServer(m_serverName);
    if(!server->listen(m_serverName)){
        qWarning()<<"Failed to start server:"<<server->errorString();
    }
}

bool SingleApplication::isRunning(){
    return m_isRunning;
}

void SingleApplication::handleNewConnection() {

}
