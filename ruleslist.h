#ifndef RULESLIST_H
#define RULESLIST_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QListWidget>
#include <QVBoxLayout>
#include <QDebug>
#include <QDesktopWidget>

#include <memory>

#include "utils.h"
#include "config.h"
#include "ruleswindow.h"

class RulesList : public QWidget
{
    Q_OBJECT

public:
    explicit RulesList(QWidget *parent = nullptr, std::shared_ptr<Config> _config = nullptr);
    ~RulesList();
    void closeEvent(QCloseEvent *);

private:
    std::shared_ptr<Config> config;
    std::unique_ptr<RulesWindow> rules_window;
    std::unique_ptr<QListWidget> list;
    std::unique_ptr<QMenu> menu;
    std::unique_ptr<QVBoxLayout> layout;

    void initList();
    QString loadLine(const QJsonObject, int);

signals:
    void closeWindowSignal();

private slots:
    void showListMenu(QPoint);
    void online_edit();
    void online_insert();
    void online_delete();
};

#endif // RULESLIST_H
