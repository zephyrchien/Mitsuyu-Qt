#include "config.h"

Config::Config(const QString _base_path, const QString _config_file):
    base_path(_base_path),
    config_file(_config_file)
{
    loadConfig();
    setConfigDefaultValue("log","info");
    setConfigDefaultValue("local","127.0.0.1:1080");
    setConfigDefaultValue("tls","true");
    setConfigDefaultValue("tls_verfiy","true");
    setConfigDefaultValue("compress","true");
}

Config::~Config()
{

}

bool Config::loadConfig()
{
    auto fp = std::make_unique<QFile>(base_path + config_file);
    if (!fp->exists()) return false;
    if (!fp->open(QIODevice::ReadOnly|QIODevice::Text)) return false;
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(fp->readAll(),&err);
    if (err.error != QJsonParseError::NoError || doc.isNull() || !doc.isObject())
        return false;
    config = doc.object();
    return true;
}

bool Config::dumpConfig()
{
    auto fp = std::make_unique<QFile>(base_path + config_file);
    if (!fp->open(QIODevice::WriteOnly|QIODevice::Text)) return false;
    QJsonDocument doc;
    doc.setObject(config);
    fp->write(doc.toJson(QJsonDocument::Compact));
    return true;
}

QJsonObject& Config::getConfig()
{
    return config;
}

QString Config::getBasePath()
{
    return base_path;
}

QString Config::getConfigFile()
{
    return base_path + config_file;
}

void Config::setConfigValue(const QString key, const QString value)
{
    if (!key.isEmpty() && !value.isEmpty())
    {
        config[key] = value;
    }
}

void Config::setConfigDefaultValue(const QString key, const QString value)
{
    if (!config.contains(key))
    {
        config[key] = value;
    }
}
