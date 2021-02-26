#include "configwindow.h"
#include "ui_configwindow.h"

ConfigWindow::ConfigWindow(QWidget *parent, std::shared_ptr<Config> _config) :
    QWidget(parent),
    ui(new Ui::ConfigWindow),
    config(_config)
{
    setAttribute(Qt::WA_QuitOnClose,false);
    ui->setupUi(this);
}

ConfigWindow::~ConfigWindow()
{
    delete ui;
}

void ConfigWindow::closeEvent(QCloseEvent *)
{
    emit closeWindowSignal();
}

void ConfigWindow::on_btn_rst_clicked()
{
    ui->edit_addr->clear();
    ui->edit_sname->clear();
    ui->edit_sni->clear();
    ui->check_verify->setChecked(false);
    ui->check_compress->setChecked(false);
}

void ConfigWindow::on_btn_ok_clicked()
{
    QJsonObject& c = config->getConfig();
    c["remote"] = ui->edit_addr->text();
    c["service_name"] = ui->edit_sname->text();
    c["tls_sni"] = ui->edit_sni->text();
    c["tls_verify"] = ui->check_verify->isChecked() ? "true" : "false";
    c["compress"] = ui->check_compress->isChecked() ? "true" : "false";
    config->dumpConfig();
    this->close();
}

void ConfigWindow::on_edit_addr_editingFinished()
{
    ui->edit_sni->setText(ui->edit_addr->text().split(":")[0]);
}
