#ifndef QOPC_H
#define QOPC_H

#endif // QOPC_H

#include <QObject>
#include <QString>
#include <QtNetwork>
#include <QUrl>

class QOPC : public QObject
{
    Q_OBJECT
public:
    QOPC() {}
    virtual ~QOPC() {}

    void getConnectMode()
    {
        QString path = "/get_connectmode.cgi";
        execCommand(path);
    }

    void execCommand(QString &path)
    {
        QUrl url("http://192.168.0.10");
        url.setPath(path);
        QNetworkRequest request(url);
        request.setRawHeader("User-Agent", "OlympusCameraKit");

        connect(&qnam, &QNetworkAccessManager::finished, this, &QOPC::httpfinished );
        qDebug() << url.toString();
        reply = qnam.get(request);
    }

private slots:
    void httpfinished()
    {
        qDebug() << "finished.";
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << QVariant(statusCode).toString();
        if(reply->error() != QNetworkReply::NoError) return;
        qDebug() << reply->url().toString();
        QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
        qDebug() << contentType;

        qDebug() << QString::fromUtf8(reply->readAll());
    }
private:
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
};
