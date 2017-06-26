#ifndef QOPCEVENTCLIENT_H
#define QOPCEVENTCLIENT_H

#endif // QOPCEVENTCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QMessageBox>

class QOPCEventClient : public QObject {
    Q_OBJECT

public:
    explicit QOPCEventClient(QObject *parent = 0) : QObject(parent)
    {
        connect(&socket, &QTcpSocket::connected, this, &QOPCEventClient::sendRequest);
        connect(&socket, &QTcpSocket::disconnected, this, &QOPCEventClient::closeConnection);
        connect(&socket, &QTcpSocket::readyRead, this, &QOPCEventClient::updateMessage);
        connect(&socket, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), this, &QOPCEventClient::error);
    }
    virtual ~QOPCEventClient() {}

    void connectToServer(QHostAddress host, qint16 port)
    {
        qDebug() << "connect to " + host.toString() + ", port:" + port;
        socket.connectToHost(host, port);
    }

    QString message;
signals:
    void done();
    void received();

private slots:
    void sendRequest()
    {
        qDebug() << "event notification port opened.";
    }
    void updateMessage()
    {
        message.clear();
        qDebug() << "received";
        QDataStream in(&socket);
        in.setVersion(QDataStream::Qt_4_1);
        in.setByteOrder(QDataStream::BigEndian);
        int messageLength = socket.bytesAvailable();
        char* rawMessage = new char[messageLength];

        in.readRawData(rawMessage, messageLength);

        quint8 appId = rawMessage[0];
        quint8 eventId = rawMessage[1];
        quint16 eventBodyLength = rawMessage[2] << sizeof(rawMessage[2]) | rawMessage[3];

        qDebug() << "appId: " << appId;
        qDebug() << "eventId" << eventId;
        qDebug() << "eventBodyLength " << eventBodyLength;
        if (eventBodyLength != 0)
        {
            message = QString(&rawMessage[4]);
        }
        delete rawMessage;
        emit received();

    }
    void error()
    {
        message = socket.errorString();
        qDebug() << message;
        socket.close();
        emit done();
    }

    void closeConnection()
    {
        socket.close();
        emit done();
    }

private:
    QTcpSocket socket;
};
