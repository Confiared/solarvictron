#ifndef LOCALSERVER_H
#define LOCALSERVER_H

#include <QLocalServer>

class LocalServer : public QLocalServer
{
    Q_OBJECT
public:
    static QString socketPath;
    LocalServer(QObject *parent = nullptr);
    void newClient();
    void newData();
    void newDataParse(QLocalSocket *socket);
};

#endif // LOCALSERVER_H
