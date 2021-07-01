#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    api_token(QUuid::createUuid().toString()),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->conn_logs->setWordWrapMode(QTextOption::NoWrap);
    ui->conn_logs->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->conn_logs->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // config
    config = std::make_shared<Config>(QDir::currentPath() + "/");
    // core process
    QAction *start_core = new QAction("boot",this);
    QAction *stop_core = new QAction("kill",this);
    ui->menu_mitsuyux->addAction(start_core);
    ui->menu_mitsuyux->addAction(stop_core);
    start_core->setStatusTip("boot");
    stop_core->setStatusTip("kill");
    connect(start_core,SIGNAL(triggered()),this,SLOT(oncore_start()));
    connect(stop_core,SIGNAL(triggered()),this,SLOT(oncore_stop()));
    // menubar->config
    QAction *config_log = new QAction("log",this);
    QAction *config_core = new QAction("core",this);
    QAction *config_local = new QAction("local",this);
    QAction *config_server = new QAction("server",this);
    ui->menu_config->addAction(config_log);
    ui->menu_config->addAction(config_core);
    ui->menu_config->addAction(config_local);
    ui->menu_config->addAction(config_server);
    config_log->setStatusTip("set log level");
    config_core->setStatusTip("load mitsuyu core");
    config_local->setStatusTip("set local client");
    config_server->setStatusTip("set server");
    connect(config_log,SIGNAL(triggered()),this,SLOT(onconfig_log()));
    connect(config_core,SIGNAL(triggered()),this,SLOT(onconfig_core()));
    connect(config_local,SIGNAL(triggered()),this,SLOT(onconfig_local()));
    connect(config_server,SIGNAL(triggered()),this,SLOT(onconfig_server()));
    // menubar->advance
    QAction *advance_api = new QAction("api",this);
    QAction *advance_rules = new QAction("rules",this);
    QAction *advance_reuse = new QAction("reuse",this);
    QAction *advance_limit = new QAction("limit",this);
    QAction *advance_padding = new QAction("padding",this);
    ui->menu_advance->addAction(advance_api);
    ui->menu_advance->addAction(advance_rules);
    ui->menu_advance->addAction(advance_reuse);
    ui->menu_advance->addAction(advance_limit);
    ui->menu_advance->addAction(advance_padding);
    advance_api->setStatusTip("set api address");
    advance_rules->setStatusTip("set rules");
    advance_limit->setStatusTip("set speed limit");
    advance_reuse->setStatusTip("set connection reuse");
    advance_padding->setStatusTip("set packet padding");
    connect(advance_api,SIGNAL(triggered()),this,SLOT(onconfig_api()));
    connect(advance_rules,SIGNAL(triggered()),this,SLOT(onconfig_rules()));
    connect(advance_reuse,SIGNAL(triggered()),this,SLOT(onconfig_reuse()));
    connect(advance_limit,SIGNAL(triggered()),this,SLOT(onconfig_limit()));
    connect(advance_padding,SIGNAL(triggered()),this,SLOT(onconfig_padding()));
    // statusbar
    ui->statusbar->showMessage("ready");
}

MainWindow::~MainWindow()
{
    delete ui;
    config.reset();
    core_process.reset();
    config_window.reset();
}

void MainWindow::oncore_start()
{
    if (core_process && core_process->isOpen())
    {
        oncore_stop();
        return oncore_start();
    }
    auto c = config->getConfig();
    if (!c.contains("core"))
    {
        ui->statusbar->showMessage("mitsuyu-core unspecified");
        return;
    }
    QString core_cmd = c.value("core").toString();
    QStringList core_args {
        "-m","client",
        "-c",config->getConfigFile(),
        "--api",c.value("api_addr").toString(),
        "--token",api_token
    };
    // tips
    const int tip_title_len = -12;
    ui->logs->append("prepare to start core process..");
    ui->logs->append(QString("%1: %2").arg("directory",tip_title_len).arg(config->getBasePath()));
    ui->logs->append(QString("%1: %2").arg("command",tip_title_len).arg(core_cmd));
    for (int i = 0, l = core_args.length(); i < l; i++)
    {
        ui->logs->append(QString("%1: %2")
            .arg(QString("argument[%1]").arg(i+1),tip_title_len)
            .arg(core_args[i]));
    }
    QStringList keys = c.keys();
    QString key;
    for (int i = 0, j = 0, l = keys.count(); i < l; i++)
    {
        key = keys[i];
        if (key == "core" || key == "strategy" || key == "api_addr") continue;
        ui->logs->append(QString("%1: %2=%3")
            .arg(QString("option  [%1]").arg(j+1),tip_title_len)
            .arg(key)
            .arg(c.value(key).toString()));
        j++;
    }
    QJsonArray rules = c.value("strategy").toArray();
    QJsonObject rule;
    QString _m, _act, _m_val, _act_val;
    for (int i = 0, l = rules.count(); i < l; i++)
    {
        rule = rules[i].toObject();
        foreach (QString m, config->match_list)
        {
            QString m_complex = config->translateMatchRule(m);
            if (rule.contains(m_complex))
            {
                _m = m;
                _m_val = rule.value(m_complex).toString();
            }
        }
        foreach (QString act, config->action_list)
        {
            if (rule.contains(act))
            {
                _act = act;
                _act_val = rule.value(act).toString();
            }
        }
        ui->logs->append(QString("%1: %2=%3 => %4=%5")
            .arg(QString("rule    [%1]").arg(i+1),tip_title_len)
            .arg(_m).arg(_m_val)
            .arg(_act).arg(_act_val));
    }

    core_process = utils::make_unique<QProcess>(this);
    core_process->setReadChannel(QProcess::StandardOutput);
    connect(core_process.get(),SIGNAL(readyReadStandardOutput()),this,SLOT(onread_output()));
    connect(core_process.get(),SIGNAL(error(QProcess::ProcessError)),this,SLOT(oncore_error(QProcess::ProcessError)));
    core_process->start(core_cmd, core_args);
    if (core_process->isOpen()) ui->statusbar->showMessage("boot");

    timer = utils::make_unique<QTimer>(this);
    netman_traffic = utils::make_unique<QNetworkAccessManager>(this);
    netman_conn = utils::make_unique<QNetworkAccessManager>(this);
    connect(timer.get(),SIGNAL(timeout()),this,SLOT(onstat_update()));
    connect(netman_traffic.get(),SIGNAL(finished(QNetworkReply*)),this,SLOT(onget_traffic_callback(QNetworkReply*)));
    connect(netman_conn.get(),SIGNAL(finished(QNetworkReply*)),this,SLOT(onget_conn_callback(QNetworkReply*)));
    timer->start(1000);
}

void MainWindow::oncore_stop()
{
    if (!core_process || !core_process->isOpen()) return;
    disconnect(core_process.get(),SIGNAL(error(QProcess::ProcessError)),this,SLOT(oncore_error(QProcess::ProcessError)));
    core_process->close();
    core_process->waitForFinished();
    core_process.reset();
    timer->stop();
    timer.reset();
    netman_traffic.reset();
    netman_conn.reset();
    ui->conn_logs->clear();
    ui->label_up->setText("0k");
    ui->label_down->setText("0k");
    ui->statusbar->showMessage("killed");
}

void MainWindow::oncore_error(QProcess::ProcessError error)
{
    oncore_stop();
    switch(error)
    {
    case QProcess::FailedToStart:
        ui->logs->append("core: failed to start");
        break;
    case QProcess::Crashed:
        ui->logs->append("core: killed");
        break;
    case QProcess::Timedout:
        ui->logs->append("core: timeout");
        break;
    case QProcess::WriteError:
        ui->logs->append("core: failed to write");
        break;
    case QProcess::ReadError:
        ui->logs->append("core: failed to read");
        break;
    case QProcess::UnknownError:
        ui->logs->append("core: unknown error");
        break;
    default:
        ui->logs->append("core: unknown error");
        break;
    }
}

void MainWindow::onread_output()
{
    static int scroll = 30;
    QString msg = QString::fromLocal8Bit(core_process->readAllStandardOutput().data()).trimmed();
    ui->logs->append(msg);
    if (ui->logs->verticalScrollBar()->isSliderDown())
        scroll = 30;
    else if (scroll == 0)
        ui->logs->moveCursor(QTextCursor::End);
    else
        scroll--;
}

void MainWindow::onstat_update()
{
    QNetworkRequest r_traffic, r_conn;
    r_traffic.setRawHeader("token",api_token.toUtf8());
    r_conn.setRawHeader("token",api_token.toUtf8());
    r_traffic.setUrl(config->api_url_traffic);
    r_conn.setUrl(config->api_url_conn);
    netman_traffic->get(r_traffic);
    netman_conn->get(r_conn);
}

void MainWindow::onconfig_log()
{
    QStringList levels;
    levels << "none" << "debug" << "info" << "error";
    const QString ps = "select log level";
    const int default_level = levels.indexOf(config->getConfig().value("log").toString());
    bool ok;
    QString log_level = QInputDialog::getItem(this,"log",ps,levels,default_level,false,&ok);
    if (!ok || log_level.isEmpty()) return;
    config->getConfig()["log"] = log_level;
    config->dumpConfig();
}

void MainWindow::onconfig_core()
{
    const QString title = "select mitsuyu-core";
    const QString filter = "bin (*.exe)";
    QString core_path = QFileDialog::getOpenFileName(this,title,config->getBasePath(),filter);
    if (core_path.isEmpty()) return;
    config->getConfig()["core"] = core_path;
    config->dumpConfig();
    this->ui->statusbar->showMessage("core: " + core_path);
}

void MainWindow::onconfig_local()
{
    bool ok;
    const QString ps = "listen address:";
    const QString default_local = config->getConfig().value("local").toString();
    QString local = QInputDialog::getText(this,"local",ps,QLineEdit::Normal,default_local,&ok);
    if (!ok || local.isEmpty()) return;
    config->getConfig()["local"] = local;
    config->dumpConfig();
    this->ui->statusbar->showMessage("local: " + local);
}

void MainWindow::onconfig_server()
{
    config_window = utils::make_unique<ConfigWindow>(nullptr,config);
    config_window->setGeometry(QStyle::alignedRect(
        Qt::LeftToRight, Qt::AlignCenter, config_window->size(),
        this->geometry()));
    config_window->show();
    connect(config_window.get(),&ConfigWindow::closeWindowSignal,[this](){
        disconnect(this->config_window.get());
        this->config_window.reset();
        this->ui->statusbar->showMessage("done");
    });
}

void MainWindow::onconfig_api()
{
    bool ok;
    const QString ps = "api address:";
    const QString default_api_addr = config->getConfig().value("api_addr").toString();
    QString api_addr = QInputDialog::getText(this,"api",ps,QLineEdit::Normal,default_api_addr,&ok);
    if (!ok || api_addr.isEmpty()) return;
    config->getConfig()["api_addr"] = api_addr;
    config->dumpConfig();
    this->ui->statusbar->showMessage("api address: " + api_addr);
}

void MainWindow::onconfig_rules()
{
    rules_list = utils::make_unique<RulesList>(nullptr, config);
    rules_list->setGeometry(QStyle::alignedRect(
        Qt::LeftToRight, Qt::AlignCenter, rules_list->size(),
        this->geometry()));
    rules_list->show();
    connect(rules_list.get(),&RulesList::closeWindowSignal,[this](){
        disconnect(this->rules_list.get());
        this->rules_list.reset();
        this->ui->statusbar->showMessage("done");
    });
}

void MainWindow::onconfig_reuse()
{
    bool ok;
    const QString ps = "reuse connection, timeout/maxsize(sec):";
    const QString default_reuse_timeout = config->getConfig().value("reuse_timeout").toString();
    const QString default_reuse_maxsize = config->getConfig().value("reuse_maxsize").toString();
    QString reuse = QInputDialog::getText(
                this,"reuse",ps,
                QLineEdit::Normal,QString("%1/%2").arg(default_reuse_timeout).arg(default_reuse_maxsize),&ok
                );
    if (!ok || reuse.isEmpty()) return;
    QStringList params = reuse.split("/");
    if (params.count() != 2) return;
    QString timeout = params[0].trimmed();
    QString maxsize = params[1].trimmed();
    config->getConfig()["reuse_timeout"] = timeout;
    config->getConfig()["reuse_maxsize"] = maxsize;
    config->dumpConfig();
    this->ui->statusbar->showMessage(QString("reuse : %1/%2").arg(timeout).arg(maxsize));
}

void MainWindow::onconfig_limit()
{
    bool ok;
    const QString ps = "speed limit, upload/download(kb):";
    const QString default_up_limit = config->getConfig().value("upload_limit").toString();
    const QString default_down_limit = config->getConfig().value("download_limit").toString();
    QString limit = QInputDialog::getText(
                this,"limit",ps,
                QLineEdit::Normal,QString("%1/%2").arg(default_up_limit).arg(default_down_limit),&ok
                );
    if (!ok || limit.isEmpty()) return;
    QStringList up_down = limit.split("/");
    if (up_down.count() != 2) return;
    QString up = up_down[0].trimmed();
    QString down = up_down[1].trimmed();
    config->getConfig()["upload_limit"] = up;
    config->getConfig()["download_limit"] = down;
    config->dumpConfig();
    this->ui->statusbar->showMessage(QString("speed limit: %1/%2").arg(up).arg(down));
}

void MainWindow::onconfig_padding()
{
    bool ok;
    const QString ps = "packet size, no less than(byte):";
    const QString default_padding = config->getConfig().value("padding").toString();
    QString padding = QInputDialog::getText(this,"padding",ps,QLineEdit::Normal,default_padding,&ok);
    if (!ok || padding.isEmpty()) return;
    config->getConfig()["padding"] = padding;
    config->dumpConfig();
    this->ui->statusbar->showMessage("use padding when packet size < " + padding);
}

void MainWindow::onget_traffic_callback(QNetworkReply *reply)
{

    static quint64 up_traffic = 0;
    static quint64 down_traffic = 0;
    if (reply->error() != QNetworkReply::NoError) return;
    QStringList data = QString(reply->readAll()).split(',');
    if (data.count() != 2) return;
    quint64 up = data[0].trimmed().toUInt();
    quint64 down = data[1].trimmed().toUInt();
    ui->label_up->setText(QString("%1k").arg((up - up_traffic)/1024));
    ui->label_down->setText(QString("%1k").arg((down - down_traffic)/1024));
    ui->label_upload->setText(QString("%1m").arg(up/(2<<20)));
    ui->label_download->setText(QString("%1m").arg(down/(2<<20)));
    up_traffic = up;
    down_traffic = down;
}

void MainWindow::onget_conn_callback(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) return;
    QStringList data = QString(reply->readAll()).split('\n');
    for (int i = 0, len = data.count(); i < len; i++)
    {
        int l = data[i].length();
        if (l > 16)
            data[i].replace(14,l-14,"..");
    }
    data.sort();
    ui->conn_logs->setText(data.join('\n'));
}
