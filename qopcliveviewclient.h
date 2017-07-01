#ifndef QOPCLIVEVIEWCLIENT_H
#define QOPCLIVEVIEWCLIENT_H

#endif // QOPCLIVEVIEWCLIENT_H

#include <QUdpSocket>
#include <QDataStream>
#include <QBitArray>
#include <QImage>

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

    void abort()
    {
        socket.abort();
    }

signals:
    void started();
    void jpgFrameUpdated(QImage frame);
private slots:
    void processDatagram()
    {
        QByteArray datagram;
        const qint32 rtpHeaderLength = 12;
        const qint32 extensionHeaderByteLength = 4;
        while(socket.hasPendingDatagrams())
        {
            datagram.resize(socket.pendingDatagramSize());
            socket.readDatagram(datagram.data(), datagram.size());

            // 拡張ヘッダを持つかどうか
            bool hasExtensionHeader = datagram[0] & 0x10;
            if(hasExtensionHeader)
            {
                jpgFrame.loadFromData(jpgBuffer);
                emit jpgFrameUpdated(jpgFrame);

                int extensionWordLength = datagram.data()[14] << sizeof(char) | datagram.data()[15];
                int extensionByteLength = extensionWordLength * 4;
                int headerSize = rtpHeaderLength + extensionByteLength + extensionHeaderByteLength;

                // 拡張ヘッダを新たなQByteArrayにして処理
                processExtensionHeader(QByteArray(&datagram.data()[rtpHeaderLength], extensionHeaderByteLength + extensionByteLength));
                // ライブビュー画像の新たなバッファを作る
                jpgBuffer = QByteArray(&datagram.data()[headerSize], datagram.size() - headerSize);
            }
            else
            {
                jpgBuffer.append(&datagram.data()[rtpHeaderLength], datagram.size() - rtpHeaderLength);
            }
        }
    }

private:
    void processExtensionHeader(QByteArray extensionHeader)
    {
        // 拡張ヘッダの内容を構造体に入れる
    }

    QUdpSocket socket;
    QImage jpgFrame;
    QByteArray jpgBuffer;
};
