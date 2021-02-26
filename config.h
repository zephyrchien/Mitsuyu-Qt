#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <memory>

class Config
{
public:
    Config(const QString _base_path = "./", const QString _config_file = ".mitsuyux");
    ~Config();
    bool loadConfig();
    bool dumpConfig();
    QString getBasePath();
    QString getConfigFile();
    QJsonObject& getConfig();
    void setConfigValue(const QString key, const QString value);
    void setConfigDefaultValue(const QString key, const QString value);

private:
    const QString base_path;
    const QString config_file;
    QJsonObject config;
};

#endif // CONFIG_H
