#include "ruleswindow.h"
#include "ui_ruleswindow.h"

RulesWindow::RulesWindow(QWidget *parent, std::shared_ptr<Config> _config, int _init_index) :
    QWidget(parent),
    init_index(_init_index),
    ui(new Ui::RulesWindow),
    config(_config)
{
    setWindowIcon(QIcon(":/icon/mitsuyu/img/water.jpg"));
    setAttribute(Qt::WA_QuitOnClose,false);
    ui->setupUi(this);
    initRulesWindow();
}

RulesWindow::~RulesWindow()
{
    delete ui;
}

void RulesWindow::closeEvent(QCloseEvent *)
{
    emit closeWindowSignal();
}

void RulesWindow::initRulesWindow()
{
    ui->box_match->addItems(config->match_list);
    ui->box_action->addItems(config->action_list);
    loadRule();
}

void RulesWindow::on_btn_ok_clicked()
{
    QString m = config->match_list[ui->box_match->currentIndex()];
    QString act = config->action_list[ui->box_action->currentIndex()];
    QString m_val = ui->edit_match->text();
    QString act_val = ui->edit_action->text();

    if (m_val.isEmpty() || (act != "block" && act_val.isEmpty())) return;
    QJsonObject r;
    r.insert(config->translateMatchRule(m),m_val);
    r.insert(act, act == "block" ? "true" : act_val);
    QJsonArray& rules = config->getRules();
    if (init_index == -1)
        rules.append(r);
    else
        rules[init_index] = r;
    config->dumpConfig();
    this->close();
}

void RulesWindow::on_btn_rst_clicked()
{
    loadRule();
}

void RulesWindow::loadRule()
{
    if (init_index != -1)
    {
        const QJsonArray& rules = config->getRules();
        QJsonObject rule = rules[init_index].toObject();
        foreach (QString m, config->match_list)
        {
            QString m_complex = config->translateMatchRule(m);
            if (rule.contains(m_complex))
            {
                ui->box_match->setCurrentIndex(ui->box_match->findText(m));
                ui->edit_match->setText(rule.value(m_complex).toString());
            }
        }
        foreach (QString act, config->action_list)
        {
            if (rule.contains(act))
            {
                ui->box_action->setCurrentIndex(ui->box_action->findText(act));
                ui->edit_action->setText(rule.value(act).toString());
            }
        }
    }
}
