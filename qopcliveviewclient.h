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

    struct FinderInformation
    {
        int frameSize;
        int afStatus;
        QPair<int, int> afFrameCoord; // <x, y>
        QPair<int, int> afFrameSize;    // <width, height>
        bool isMediaExists; // 記録メディアがあるか
        bool isMediaVacant; // メディアに空きがあるか
        bool isMediaProtected;  // カードプロテクト状態
        bool isMediaError;  // メディアにエラーがあるか
        bool isMediaWriting;    // メディアに書き込み中か
        int orientation;    // カメラの傾き（EXIFのorientationタグと同じ）
        int numberOfRecordable; // 撮影可能枚数
        quint16 minSSNumerator;     // シャッター速度最遅値の分子
        quint16 minSSDenominator;   // シャッター速度最遅値の分母
        quint16 maxSSNumerator;     // シャッター速度最速値の分子
        quint16 maxSSDenominator;   // シャッター速度最遅値の分母
        quint16 currentSSNumerator; // シャッター速度現在値の分子
        quint16 currentSSDenominator;   // シャッター速度現在値の分子
        int maxApertureValue;   // 最小F値
        int minApertureValue;   // 開放F値
        int currentApertureValue; // 現在のF値
        signed int maxExpCompensation; // 露出補正の上限値
        signed int minExpCompensation; // 露出補正の下限値
        signed int currentExpCompansation; // 露出補正の現在値
        int currentISOValue;
        bool isISOAuto;
        bool extendedISOWarning;
    };
signals:
    void started();
    void jpgFrameUpdated(QImage frame);
    void finderInfoUpdated(FinderInformation finderInfo);
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

                // 拡張ヘッダを新たなQByteArrayにして処理(拡張ヘッダに含まれるのは個別情報のみ)
                QByteArray extensionHeaderByteArray = QByteArray(&datagram.data()[rtpHeaderLength  + extensionHeaderByteLength], extensionByteLength);
                processExtensionHeader(&extensionHeaderByteArray);
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
    // 拡張ヘッダの内容をオブジェクトにコピーするメソッド
    // extensionHeaderは拡張ヘッダ部の先頭4バイトを抜いたもの
    void processExtensionHeader(QByteArray* extensionHeaderArray)
    {
        QDataStream extensionHeader;
        extensionHeader << extensionHeaderArray->size() << extensionHeaderArray->data();
        quint16 functionId;
        quint16 length;
        FinderInformation finderInfo;
        while(!extensionHeader.atEnd())
        {
            extensionHeader >> functionId;
            extensionHeader >> length;
            int byteLength = length * 4;
            switch (functionId)
            {
                case 8: // シャッター速度
                    extensionHeader >> finderInfo.minSSNumerator;
                    extensionHeader >> finderInfo.minSSDenominator;
                    extensionHeader >> finderInfo.maxSSNumerator;
                    extensionHeader >> finderInfo.maxSSDenominator;
                    extensionHeader >> finderInfo.currentSSNumerator;
                    extensionHeader >> finderInfo.currentSSDenominator;
                    break;
                case 9: // F値
                    extensionHeader >> finderInfo.maxApertureValue;
                    extensionHeader >> finderInfo.minApertureValue;
                    extensionHeader >> finderInfo.currentApertureValue;
                    break;
                default:
                    extensionHeader.skipRawData(byteLength);
            }
        }

        emit finderInfoUpdated(finderInfo);
    }

    QUdpSocket socket;
    QImage jpgFrame;
    QByteArray jpgBuffer;
};
