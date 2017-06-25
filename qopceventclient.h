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

    void connectToServer(QHostAddress &host, qint16 &port)
    {
        socket.connectToHost(host, port);
        nextBlockSize = 0;
    }

    QString message;
signals:
    void done();

private slots:
    void sendRequest() {}
    void updateMessage()
    {
        QDataStream in(&socket);
        in.setVersion(QDataStream::Qt_4_1);

        forever {
            if (nextBlockSize == 0) {
                if (socket.bytesAvailable() < sizeof(quint16))
                    break;
                in >> nextBlockSize;
            }
            if (nextBlockSize == 0xFFFF) {
                socket.close();
                break;
            }
            if (socket.bytesAvailable() < nextBlockSize)
                break;

            in >> message;

            nextBlockSize = 0;
        }
    }
    void error()
    {
        message = socket.errorString();
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
    qint16 nextBlockSize;
};
