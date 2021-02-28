#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
    QAction *advance_rules = new QAction("rules",this);
    ui->menu_advance->addAction(advance_rules);
    advance_rules->setStatusTip("set rules");
    connect(advance_rules,SIGNAL(triggered()),this,SLOT(onconfig_rules()));
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
    QStringList core_args{"-m","client","-c",config->getConfigFile()};
    // tips
    ui->logs->append("prepare to start core process..");
    ui->logs->append("directory: " + config->getBasePath());
    ui->logs->append("command: " + core_cmd);
    for (int i = 0, l = core_args.length(); i < l; i++)
    {
        ui->logs->append(QString("argument[%1]: ").arg(i+1) + core_args[i]);
    }
    QStringList keys = c.keys();
    QString key;
    for (int i = 0, j = 0, l = keys.count(); i < l; i++)
    {
        key = keys[i];
        if (key == "core" || key == "strategy") continue;
        ui->logs->append(QString("option[%1]: %2=%3").arg(j+1).arg(key).arg(c.value(key).toString()));
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
        ui->logs->append(QString("rule[%1]: %2=%3 => %4=%5").arg(i+1).arg(_m).arg(_m_val).arg(_act).arg(_act_val));
    }

    core_process = std::make_unique<QProcess>(this);
    core_process->setReadChannel(QProcess::StandardOutput);
    connect(core_process.get(),SIGNAL(readyReadStandardOutput()),this,SLOT(onread_output()));
    connect(core_process.get(),SIGNAL(error(QProcess::ProcessError)),this,SLOT(oncore_error(QProcess::ProcessError)));
    core_process->start(core_cmd, core_args);
    if (core_process->isOpen()) ui->statusbar->showMessage("boot");
}

void MainWindow::oncore_stop()
{
    if (!core_process || !core_process->isOpen()) return;
    core_process->close();
    core_process->waitForFinished();
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
    QString msg = QString::fromLocal8Bit(core_process->readAllStandardOutput().data()).trimmed();
    ui->logs->append(msg);
    ui->logs->moveCursor(QTextCursor::End);
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
    config_window = std::make_unique<ConfigWindow>(nullptr,config);
    config_window->show();
    connect(config_window.get(),&ConfigWindow::closeWindowSignal,[this](){
        disconnect(this->config_window.get());
        this->config_window.reset();
        this->ui->statusbar->showMessage("done");
    });
}

void MainWindow::onconfig_rules()
{
    rules_list = std::make_unique<RulesList>(nullptr, config);
    rules_list->show();
    connect(rules_list.get(),&RulesList::closeWindowSignal,[this](){
        disconnect(this->rules_list.get());
        this->rules_list.reset();
        this->ui->statusbar->showMessage("done");
    });
}
