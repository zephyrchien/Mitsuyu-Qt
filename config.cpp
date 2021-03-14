#include "config.h"

Config::Config(const QString _base_path, const QString _config_file):
    match_list(QStringList{"ip","port","prefix","suffix","keyword"}),
    action_list(QStringList{"dns","next","block"}),
    match_list_complex(QStringList{"ip_range","port_range","domain_prefix","domain_suffix","domain_contain"}),
    base_path(_base_path),
    config_file(_config_file)
{
    loadConfig();
    setConfigDefaultValue("log","info");
    setConfigDefaultValue("local","127.0.0.1:1080");
    setConfigDefaultValue("tls","true");
    setConfigDefaultValue("tls_verify","true");
    setConfigDefaultValue("compress","true");
    setConfigDefaultValue("padding","0");
    setConfigDefaultValue("reuse_timeout","0");
    setConfigDefaultValue("reuse_maxsize","0");
    setConfigDefaultValue("upload_limit","0");
    setConfigDefaultValue("download_limit","0");
    setConfigDefaultValue("api_addr","127.0.0.1:10000");
    setConfigDefaultValue("strategy",QJsonArray());
    rules = config.value("strategy").toArray();
    QString api_base_path = QString("http://%1/%2")
            .arg(config.value("api_addr").toString())
            .arg(config.value("service_name").toString());
    api_url_conn = QUrl(api_base_path + "/connection");
    api_url_traffic = QUrl(api_base_path + "/traffic");
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
    config["strategy"] = rules;
    doc.setObject(config);
    fp->write(doc.toJson(QJsonDocument::Compact));
    return true;
}

QJsonObject& Config::getConfig()
{
    return config;
}

QJsonArray& Config::getRules()
{
    return rules;
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

void Config::setConfigDefaultValue(const QString key, const QJsonArray value)
{
    if (!config.contains(key))
    {
        config[key] = value;
    }
}

QString Config::translateMatchRule(const QString src)
{
    for (int i = 0, l = match_list.count(); i < l; i++)
    {
        if (match_list[i] == src) return match_list_complex[i];
    }
    return "error";
}
