#ifndef QOPC_H
#define QOPC_H

#endif // QOPC_H

#include <QObject>
#include <QString>
#include <QtNetwork>
#include <QUrl>
#include <QtXml/QtXml>
#include <qopceventclient.h>

class QOPC : public QObject
{
    Q_OBJECT
public:
    explicit QOPC(QObject* parent = Q_NULLPTR): QObject(parent) {
        connect(&qnam, &QNetworkAccessManager::finished, this, &QOPC::commandfinished );
    }
    virtual ~QOPC() {}

    void negotiate()
    {
        QEventLoop loop1;
        getConnectMode(&loop1);
        loop1.exec();

        qDebug() << "get req finished.";
        if(reply->error() != QNetworkReply::NoError) return;
        QByteArray responseBody = reply->readAll();
        QDomDocument doc;
        doc.setContent(responseBody);
        qDebug() << doc.toString();
        if (doc.documentElement().tagName() != "connectmode" && doc.documentElement().text() != "OPC") return;

        // コマンド受付元切り替え
        qDebug() << "コマンド受付元切り替え";
        QEventLoop loop2;
        switchCommpath(&loop2);
        loop2.exec();
        if(reply->error() != QNetworkReply::NoError) return;

        // カメライベント通知開始
        qDebug() << "カメライベント通知開始";
        QEventLoop loop3;
        startPushEvent(&loop3);
        loop3.exec();
        if(reply->error() != QNetworkReply::NoError) return;
        connect(&eventClient, &QOPCEventClient::received, this, &QOPC::cameraEventReceived);
        eventClient.connectToServer(QHostAddress(hostAddress), eventPort.toInt());

        // 動作モード切り替え（スタンドアロン）
        qDebug() << "動作モード切り替え";
        switchCameraMode();

        // 動作モード切り替え（撮影モード）
        qDebug() << "撮影モードに切り替え";
        QEventLoop loop4;
        switchCameraMode("rec", &loop4);
        loop4.exec();
        if(reply->error() != QNetworkReply::NoError) return;

        responseBody = reply->readAll();
        doc.setContent(responseBody);
        qDebug() << doc.toString();

    }

    // 接続モード取得
    void getConnectMode(QEventLoop* loop = Q_NULLPTR)
    {
        QString path = "/get_connectmode.cgi";
        execCommand(path, loop);
    }

    // コマンド受付元切り替え
    void switchCommpath(QEventLoop* loop = Q_NULLPTR)
    {
        QString path = "/switch_commpath.cgi?path=wifi";
        execCommand(path, loop);
    }

    // カメライベント通知開始
    void startPushEvent(QEventLoop* loop = Q_NULLPTR)
    {
        QString path = "/start_pushevent.cgi?port=" + eventPort;\
        execCommand(path, loop);
    }

    // 動作モード切り替え
    void switchCameraMode(QString mode = "standalone", QEventLoop* loop = Q_NULLPTR)
    {
        QString path = "/switch_cameramode.cgi?mode=" + mode;
        execCommand(path, loop);
    }

    // コマンド実行
    // 同期通信を行う場合はQEventLoop*を渡す
    void execCommand(QString &path, QEventLoop* loop = Q_NULLPTR)
    {
        QUrl url("http://192.168.0.10");
        url.setPath(path);
        QNetworkRequest request(url);
        request.setRawHeader("User-Agent", "OlympusCameraKit");

        if (loop != Q_NULLPTR)
        {
            connect(&qnam, &QNetworkAccessManager::finished, loop, &QEventLoop::quit );
        }

        reply = qnam.get(request);
    }

private slots:
    void commandfinished()
    {
        qDebug() << "Command finished.";

        if(reply->error() != QNetworkReply::NoError) return;
        qDebug() << reply->url().toString();
        qDebug() << reply->header(QNetworkRequest::ContentTypeHeader).toString();
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << QVariant(statusCode).toString();
    }

    void cameraEventReceived()
    {
        qDebug() << eventClient.message;
    }
private:
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
    QOPCEventClient eventClient;
    const QString hostAddress = "192.168.0.10";
    const QString eventPort = "65000";
};
