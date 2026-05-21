#include "ai/OpenAiClient.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

OpenAiClient::OpenAiClient(AppConfig config) : m_config(std::move(config)) {}

QString OpenAiClient::chat(const QString& prompt) {
    const QString key = apiKey();
    if (key.isEmpty()) {
        return {};
    }

    QNetworkAccessManager manager;
    QNetworkRequest request{QUrl(endpoint())};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(key).toUtf8());

    QJsonArray messages;
    messages.append(QJsonObject{
        {"role", "system"},
        {"content", "You are PomoLyth, a concise focus planning and review assistant. Keep answers practical and short."}
    });
    messages.append(QJsonObject{
        {"role", "user"},
        {"content", prompt}
    });

    QJsonObject body;
    body.insert("model", m_config.openAiModel);
    body.insert("messages", messages);
    body.insert("temperature", 0.3);

    QNetworkReply* reply = manager.post(request, QJsonDocument(body).toJson(QJsonDocument::Compact));

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(30000);
    loop.exec();

    if (timer.isActive()) {
        timer.stop();
    } else {
        reply->abort();
        reply->deleteLater();
        return {};
    }

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return {};
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    if (!doc.isObject()) {
        return {};
    }

    const QJsonArray choices = doc.object().value("choices").toArray();
    if (choices.isEmpty()) {
        return {};
    }

    return choices.first().toObject().value("message").toObject().value("content").toString().trimmed();
}

QString OpenAiClient::endpoint() const {
    QString base = m_config.openAiBaseUrl.trimmed();
    if (base.isEmpty()) {
        base = "https://api.openai.com/v1";
    }
    while (base.endsWith('/')) {
        base.chop(1);
    }
    return base + "/chat/completions";
}

QString OpenAiClient::apiKey() const {
    const QString envName = m_config.openAiApiKeyEnv.trimmed().isEmpty()
        ? "OPENAI_API_KEY"
        : m_config.openAiApiKeyEnv.trimmed();
    return QString::fromLocal8Bit(qgetenv(envName.toLocal8Bit().constData())).trimmed();
}
