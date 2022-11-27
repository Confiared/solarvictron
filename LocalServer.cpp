#include "LocalServer.h"
#include "ProcessComm.h"
#include <QLocalSocket>
#include <QFile>
#include <QDebug>
#include <iostream>

QString LocalServer::socketPath;

LocalServer::LocalServer(QObject *parent) : QLocalServer(parent)
{
    QLocalSocket client;
    client.setServerName(LocalServer::socketPath);
    client.connectToServer(QIODevice::ReadWrite);
    client.waitForConnected();
    if(client.isOpen())
    {
        std::cerr << "Monitoring process is already started, check via: " << std::endl;
        std::cerr << "ps aux | grep solarvictron" << std::endl;
        std::cerr << "netstat -nap | grep solarvictron" << std::endl;
        abort();
    }
    listen(LocalServer::socketPath);
    if(!isListening())
    {
        QFile::remove(LocalServer::socketPath);
        if(!fullServerName().isEmpty())
            QFile::remove(fullServerName());
        listen(LocalServer::socketPath);
        if(!isListening())
        {
            std::cerr << "Unable to listen, check via: " << std::endl;
            std::cerr << "netstat -nap | grep solarvictron" << std::endl;
            abort();
        }
    }
    if(!connect(this,&QLocalServer::newConnection,this,&LocalServer::newClient))
        abort();
}

void LocalServer::newClient()
{
    QLocalSocket *socket = nextPendingConnection();
    if(socket==nullptr)
        return;
    if(!connect(socket, &QLocalSocket::readyRead, this, &LocalServer::newData))
        abort();
    if(socket->bytesAvailable())
        newDataParse(socket);
}

void LocalServer::newData()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
    if(socket==nullptr)
        return;
    newDataParse(socket);
}

void LocalServer::newDataParse(QLocalSocket *socket)
{
    const QByteArray &block=socket->readAll();
    const QString &var=QString::fromLocal8Bit(block);
    if(var=="battery")
    {
        socket->write(ProcessComm::get_battery().toLocal8Bit());
        return;
    }
    if(var=="load")
    {
        socket->write(ProcessComm::get_load().toLocal8Bit());
        return;
    }
    if(var=="solar")
    {
        socket->write(ProcessComm::get_solar().toLocal8Bit());
        return;
    }
    if(var=="voltage")
    {
        socket->write(ProcessComm::get_voltage().toLocal8Bit());
        return;
    }
    socket->write(QString("Wrong value").toLocal8Bit());
    return;
}
