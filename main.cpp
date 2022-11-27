#include <QCoreApplication>
#include <QDataStream>
#include <QLocalSocket>
#include <iostream>
#include "LocalServer.h"
#include "ProcessComm.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    const QStringList &arguments=a.arguments();
    if(arguments.size()>1)
    {
        LocalServer::socketPath=arguments.at(1);
        if(LocalServer::socketPath.isEmpty())
        {
            std::cerr << "the first arguement need be the socketpath" << std::endl;
            abort();
        }
    }
    if(arguments.size()==3)
    {
        const QString &var=arguments.last();
        if(var!="battery" && var!="load" && var!="solar" && var!="voltage")
        {
            std::cerr << "the requested value is not: battery, load, solar, voltage" << std::endl;
            return 995;
        }
        QLocalSocket client;
        client.setServerName(LocalServer::socketPath);
        client.connectToServer(QIODevice::ReadWrite);
        client.waitForConnected();
        if(!client.isOpen())
        {
            std::cerr << "Monitoring process is not started, check via: " << std::endl;
            std::cerr << "ps aux | grep solarvictron" << std::endl;
            std::cerr << "netstat -nap | grep solarvictron" << std::endl;
            std::cerr << client.errorString().toStdString() << std::endl;
            abort();
        }
        client.write(var.toLocal8Bit());
        client.waitForBytesWritten();
        client.waitForReadyRead();
        std::cout << QString::fromLocal8Bit(client.readAll()).toStdString();
        return 0;
    }
    if(arguments.size()!=5)
    {
        std::cerr << "Need arguments:" << std::endl;
        std::cerr << "solarvictron socketpath [host] [id] [password]" << std::endl;
        std::cerr << "solarvictron socketpath battery" << std::endl;
        std::cerr << "solarvictron socketpath load" << std::endl;
        std::cerr << "solarvictron socketpath solar" << std::endl;
        std::cerr << "solarvictron socketpath voltage" << std::endl;
        abort();
    }
    ProcessComm::host=arguments.at(2);
    ProcessComm::id=arguments.at(3);
    ProcessComm::password=arguments.at(4);

    LocalServer s;
    (void)s;
    ProcessComm p;
    (void)p;

    return a.exec();
}
