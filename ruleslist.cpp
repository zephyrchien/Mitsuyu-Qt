#include "ruleslist.h"

RulesList::RulesList(QWidget *parent, std::shared_ptr<Config> _config) :
    QWidget(parent),
    config(_config)
{
    setWindowTitle("stream");
    setWindowIcon(QIcon(":/icon/mitsuyu/img/stream.jpg"));
    list = std::make_unique<QListWidget>(this);
    menu = std::make_unique<QMenu>(this);
    layout = std::make_unique<QVBoxLayout>(this);

    layout->addWidget(list.get());
    setAttribute(Qt::WA_QuitOnClose,false);
    this->resize(450,300);
    this->setLayout(layout.get());
    //this->setWindowFlags(Qt::Window);

    initList();
}

RulesList::~RulesList()
{
    layout.reset();
    list.reset();
}

void RulesList::closeEvent(QCloseEvent *)
{
    emit closeWindowSignal();
}

void RulesList::initList()
{
    list->setContextMenuPolicy(Qt::CustomContextMenu);
    QAction *line_edit = new QAction("edit",this);
    QAction *line_insert = new QAction("insert",this);
    QAction *line_delete = new QAction("delete",this);
    menu->addAction(line_edit);
    menu->addAction(line_insert);
    menu->addAction(line_delete);
    connect(line_edit,SIGNAL(triggered()),this,SLOT(online_edit()));
    connect(line_insert,SIGNAL(triggered()),this,SLOT(online_insert()));
    connect(line_delete,SIGNAL(triggered()),this,SLOT(online_delete()));
    connect(list.get(),SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showListMenu(QPoint)));

    //list->setDragDropMode(QListWidget::InternalMove);
    const QJsonArray& rules = config->getRules();
    for (int i = 0, l = rules.count(); i < l; i++)
    {
        QJsonObject rule = rules[i].toObject();
        QString line = loadLine(rule,i);
        list->addItem(line);
    }
}

void RulesList::showListMenu(QPoint)
{
    menu->exec(QCursor::pos());
}

QString RulesList::loadLine(const QJsonObject rule, int i)
{
    QString _m, _act, _m_val, _act_val;
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
    return QString("%1|%2=%3|%4=%5").arg(i).arg(_m).arg(_m_val).arg(_act).arg(_act_val);
}

void RulesList::online_edit()
{
    auto items = list->selectedItems();
    if (items.isEmpty()) return;
    int row = list->row(items[0]);
    rules_window = std::make_unique<RulesWindow>(nullptr,config,row);
    rules_window->show();
    connect(rules_window.get(),&RulesWindow::closeWindowSignal,[this,items,row](){
        QJsonArray& rules = config->getRules();
        QJsonObject rule = rules[row].toObject();
        QString line = loadLine(rule,row);
        items[0]->setText(line);
        disconnect(this->rules_window.get());
        this->rules_window.reset();
    });
}

void RulesList::online_insert()
{
    rules_window = std::make_unique<RulesWindow>(nullptr,config,-1);
    rules_window->show();
    connect(rules_window.get(),&RulesWindow::closeWindowSignal,[this](){
        if (list->count() < config->getRules().count())
        {
            QJsonArray& rules = config->getRules();
            QJsonObject rule = rules.last().toObject();
            QString line = loadLine(rule,rules.count()-1);
            list->addItem(line);
        }
        disconnect(this->rules_window.get());
        this->rules_window.reset();
    });
}

void RulesList::online_delete()
{
    auto items = list->selectedItems();
    QJsonArray& rules = config->getRules();
    foreach(auto item, items)
    {
        int row = list->row(item);
        delete list->takeItem(row);
        rules.removeAt(row);
    }
    list->clear();
    for (int i = 0, l = rules.count(); i < l; i++)
    {
        QJsonObject rule = rules[i].toObject();
        QString line = loadLine(rule,i);
        list->addItem(line);
    }
    config->dumpConfig();
}
