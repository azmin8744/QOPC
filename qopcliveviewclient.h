#ifndef QOPCLIVEVIEWCLIENT_H
#define QOPCLIVEVIEWCLIENT_H

#endif // QOPCLIVEVIEWCLIENT_H

#include <QUdpSocket>
#include <QDataStream>
#include <QBitArray>

class QOPCLiveViewClient : public QObject {
    Q_OBJECT

public:
    explicit QOPCLiveViewClient(QObject *parent = 0) : QObject(parent) {}

    virtual ~QOPCLiveViewClient() {}

    void bind(const QHostAddress& address, const int &port)
    {
        qDebug() << socket.bind(port);
        connect(&socket, &QUdpSocket::readyRead, this, &QOPCLiveViewClient::processDatagram);

        if(socket.state() == socket.BoundState)
        {
            qDebug() << "bound started.";
            emit started();
        }
    }

signals:
    void started();
private slots:
    void processDatagram()
    {
        qDebug() << "readyRead.";
        QByteArray datagram;
        const quint32 rtpHeaderSize = 32;


        while(socket.hasPendingDatagrams())
        {
            datagram.resize(socket.pendingDatagramSize());
            socket.readDatagram(datagram.data(), datagram.size());

            // RTPヘッダを取り出す
            QDataStream stream(&datagram, QIODevice::ReadWrite);
            QBitArray rtpHeader;
            stream << rtpHeaderSize << datagram;
            stream.device()->reset();
            stream >> rtpHeader;

            qDebug() << "output bits" << rtpHeader;
        }
    }

private:
    QUdpSocket socket;

};
