#ifndef QOPC_H
#define QOPC_H

#endif // QOPC_H

#include <QObject>
#include <QString>
#include <QtNetwork>
#include <QUrl>
#include <QtXml/QtXml>

class QOPC : public QObject
{
    Q_OBJECT
public:
    QOPC() {}
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
        startPushEvent(loop3);
        loop3.exec();
        if(reply->error() != QNetworkReply::NoError) return;

    }

    void getConnectMode(QEventLoop* loop = Q_NULLPTR)
    {
        QString path = "/get_connectmode.cgi";
        execCommand(path, loop);
    }

    void switchCommpath(QEventLoop* loop = Q_NULLPTR)
    {
        QString path = "/switch_commpath.cgi?path=wifi";
        execCommand(path, loop);
    }

    void startPushEvent(QEventLoop* loop = Q_NULLPTR)
    {
        QString path = "/start_pushevent.cgi?port=" + eventPort;\
        execCommand(path, loop);
    }

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
        else
        {
            connect(&qnam, &QNetworkAccessManager::finished, this, &QOPC::commandfinished );
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
private:
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
    const QString eventPort = "65000";
};
