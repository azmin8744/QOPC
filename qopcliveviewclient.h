#ifndef QOPCLIVEVIEWCLIENT_H
#define QOPCLIVEVIEWCLIENT_H

#endif // QOPCLIVEVIEWCLIENT_H

#include <QUdpSocket>

class QOPCLiveViewClient : public QObject {
    Q_OBJECT

public:
    explicit QOPCLiveViewClient(QObject *parent = 0) : QObject(parent)
    {
        connect(&socket, &QUdpSoocket::readyRead, this, &QOPCLiveViewClient::updateMessage);
    }

    virtual ~QOPCLiveViewClient() {}

    bind(qint16 &port)
    {
        socket.bind(port);
    }

private slots:
    void updateMessage()
    {

    }

private:
    QUdpSocket socket;

};
