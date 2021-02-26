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
    ui->menu_mitsuyux->addAction(stop_core);
    ui->menu_mitsuyux->addAction(start_core);
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
    config_server->setStatusTip("set server server");
    connect(config_log,SIGNAL(triggered()),this,SLOT(onconfig_log()));
    connect(config_core,SIGNAL(triggered()),this,SLOT(onconfig_core()));
    connect(config_local,SIGNAL(triggered()),this,SLOT(onconfig_local()));
    connect(config_server,SIGNAL(triggered()),this,SLOT(onconfig_server()));
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
    QString core_arg = " -m client -c " + config->getConfigFile();
    core_process = std::make_unique<QProcess>(this);
    core_process->setReadChannel(QProcess::StandardOutput);
    connect(core_process.get(),SIGNAL(readyReadStandardOutput()),this,SLOT(onread_output()));
    connect(core_process.get(),SIGNAL(error(QProcess::ProcessError)),this,SLOT(oncore_error(QProcess::ProcessError)));
    core_process->start(core_cmd + core_arg);
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
    const int default_level = 2;
    bool ok;
    QString log_level = QInputDialog::getItem(this,"log",ps,levels,default_level,false,&ok);
    if (!ok || log_level.isEmpty()) return;
    config->getConfig()["log"] = log_level;
    config->dumpConfig();
    ui->logs->append("set log level: " + log_level);
}

void MainWindow::onconfig_core()
{
    const QString title = "select mitsuyu-core";
    const QString filter = "bin (*.exe)";
    QString core_path = QFileDialog::getOpenFileName(this,title,config->getBasePath(),filter);
    if (core_path.isEmpty()) return;
    config->getConfig()["core"] = core_path;
    config->dumpConfig();
    ui->logs->append("new core: " + core_path);
}

void MainWindow::onconfig_local()
{
    bool ok;
    const QString ps = "listen address:";
    const QString default_local = "127.0.0.1:1080";
    QString local = QInputDialog::getText(this,"local",ps,QLineEdit::Normal,default_local,&ok);
    if (!ok || local.isEmpty()) return;
    config->getConfig()["local"] = local;
    config->dumpConfig();
    ui->logs->append("set local address: " + local);
}

void MainWindow::onconfig_server()
{
    config_window = std::make_unique<ConfigWindow>(nullptr,config);
    config_window->show();
    connect(config_window.get(),&ConfigWindow::closeWindowSignal,[this](){
        disconnect(this->config_window.get());
        this->config_window.reset();
        this->ui->statusbar->showMessage("close");
    });
}
