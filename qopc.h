#ifndef QOPC_H
#define QOPC_H

#endif // QOPC_H

#include <QObject>
#include <QString>
#include <QtNetwork>
#include <QUrl>
#include <QtXml/QtXml>
#include <qopceventclient.h>
#include <qopcliveviewclient.h>

class QOPC : public QObject
{
    Q_OBJECT
public:
    explicit QOPC(QObject* parent = Q_NULLPTR): QObject(parent) {
        connect(&qnam, &QNetworkAccessManager::finished, this, &QOPC::commandfinished );
        connect(&liveViewClient, &QOPCLiveViewClient::jpgFrameUpdated, this, &QOPC::updateLiveViewFrame);
    }

    virtual ~QOPC() {}

    void negotiate()
    {
        QEventLoop loop1;
        getConnectMode(&loop1);
        loop1.exec();

        qDebug() << "get req finished.";
        if(reply->error() != QNetworkReply::NoError) return;
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
        QEventLoop loop4;
        switchCameraMode("standalone", &loop4);
        loop4.exec();


        // 動作モード切り替え（撮影モード）
        qDebug() << "撮影モードに切り替え";
        QEventLoop loop5;
        QUrlQuery param;
        param.addQueryItem("lvqty", "0640x0480");
        switchCameraMode("rec", &loop5, &param);
        loop5.exec();
        if(reply->error() != QNetworkReply::NoError) return;
        // カメラモード切替通知がきたらライブビュー転送を開始する

        connect(&eventClient, &QOPCEventClient::cameramode, this, &QOPC::startLiveView);
        //connect(&liveViewClient, &QOPCLiveViewClient::started, [=](){QObject::disconnect(&eventClient, SIGNAL(QOPCEventClient::cameramode()), this, 0);});
    }

    void closeConnection()
    {
        stopLiveView();
        stopPushEvent();
        liveViewClient.abort();
        eventClient.disconnectFromServer();
        switchCameraMode();
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
        QString path = "/start_pushevent.cgi";\
        QUrlQuery query;
        query.addQueryItem("port", eventPort);
        execCommand(path, loop, &query);
    }

    // 動作モード切り替え
    void switchCameraMode(QString mode = "standalone",QEventLoop* loop = Q_NULLPTR, QUrlQuery* param = Q_NULLPTR)
    {
        QString path = "/switch_cameramode.cgi";
        QUrlQuery query;
        query.addQueryItem("mode", mode);
        if(param != Q_NULLPTR)
        {
            QList<QPair<QString, QString>> option = param->queryItems();
            for(QPair<QString, QString> entry : option)
            {
                query.addQueryItem(entry.first, entry.second);
            }
        }
        execCommand(path, loop, &query);
    }

    // イベント通知終了
    void stopPushEvent(QEventLoop* loop = Q_NULLPTR)
    {
        QString path = "/stop_pushevent.cgi";
        execCommand(path, loop);
    }

    // 撮影制御
    void execTakeMotion(QUrlQuery* query, QEventLoop* loop = Q_NULLPTR)
    {
        QString path = "/exec_takemotion.cgi";
        execCommand(path, loop, query);
    }

    // ライブビュー画像転送停止
    void stopLiveView()
    {
        QUrlQuery query;
        query.addQueryItem("com", "stopliveview");
        execTakeMisc(&query);
    }

    // 撮影補助コマンドを送出する
    // クエリにcomは必須
    void execTakeMisc(QUrlQuery* query, QEventLoop* loop = Q_NULLPTR)
    {
        if(!query->hasQueryItem("com")) return;
        QString path = "/exec_takemisc.cgi";
        execCommand(path, loop, query);
    }

    // コマンド実行
    // 同期通信を行う場合はQEventLoop*を渡す
    void execCommand(const QString &path, QEventLoop* loop = Q_NULLPTR, const QUrlQuery *query = Q_NULLPTR)
    {
        QUrl url("http://192.168.0.10");
        url.setPath(path);
        if (query != Q_NULLPTR)
        {
            url.setQuery(query->toString());
        }

        QNetworkRequest request(url);
        request.setRawHeader("User-Agent", "OlympusCameraKit");

        if (loop != Q_NULLPTR)
        {
            connect(&qnam, &QNetworkAccessManager::finished, loop, &QEventLoop::quit );
        }

        reply = qnam.get(request);
    }

signals:
    void jpgFrameUpdated(QImage frame);
public slots:
    // 単写
    void singleShot(QPair<int, int> *afpoint = Q_NULLPTR, QEventLoop* event = Q_NULLPTR)
    {
        QUrlQuery query;
        query.addQueryItem("com", "newstarttake");
        if(afpoint != Q_NULLPTR)
        {
            QString coodinate = QString("%1x%2").arg(afpoint->first, 4, 10, QChar('0')).arg(afpoint->second, 4, 10, QChar('0'));
            query.addQueryItem("point", coodinate);
        }
        execTakeMotion(&query, event);
    }
private slots:
    void commandfinished()
    {
        qDebug() << "Command finished.";

        if(reply->error() != QNetworkReply::NoError) return;
        qDebug() << reply->url().toString();
        QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
        qDebug() << reply->header(QNetworkRequest::ContentTypeHeader).toString();

        if(contentType == "text/xml")
        {
            responseBody = reply->readAll();
            QDomDocument doc;
            doc.setContent(responseBody);
            qDebug() << doc.toString();
        }

        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << QVariant(statusCode).toString();
    }

    // ライブビュー画像転送開始
    void startLiveView()
    {
        qDebug() << "ライブビュー転送ポートオープン";
        // 転送ポートオープン
        liveViewClient.bind(QHostAddress(hostAddress), liveViewPort.toInt());

        // ライブビュー転送開始コマンド送出
        QUrlQuery query;
        query.addQueryItem("com", "startliveview");
        query.addQueryItem("port", liveViewPort);
        execTakeMisc(&query);
    }

    void cameraEventReceived()
    {
        qDebug() << eventClient.message;
    }

    void updateLiveViewFrame(QImage frame)
    {
        emit jpgFrameUpdated(frame);
    }
private:
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
    QOPCEventClient eventClient;
    QOPCLiveViewClient liveViewClient;
    QByteArray responseBody;
    const QString hostAddress = "192.168.0.10";
    const QString eventPort = "65000";
    const QString liveViewPort = "5555";
};
