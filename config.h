#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>

#include <memory>

class Config
{
public:
    Config(const QString _base_path = "./", const QString _config_file = ".mitsuyux");
    ~Config();

    const QStringList match_list;
    const QStringList action_list;
    const QStringList match_list_complex;

    QUrl api_url_traffic;
    QUrl api_url_conn;

    bool loadConfig();
    bool dumpConfig();
    QString getBasePath();
    QString getConfigFile();
    QJsonObject& getConfig();
    QJsonArray& getRules();
    void setConfigValue(const QString, const QString);
    void setConfigDefaultValue(const QString, const QString);
    void setConfigDefaultValue(const QString, const QJsonArray);
    QString translateMatchRule(const QString);

private:
    const QString base_path;
    const QString config_file;
    QJsonObject config;
    QJsonArray rules;
};

#endif // CONFIG_H
