#ifndef RULESWINDOW_H
#define RULESWINDOW_H

#include <QWidget>

#include "config.h"

namespace Ui {
class RulesWindow;
}

class RulesWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RulesWindow(QWidget *parent = nullptr, std::shared_ptr<Config> _config = nullptr, int _init_index = -1);
    ~RulesWindow();
    void closeEvent(QCloseEvent *);

private:
    const int init_index;

    Ui::RulesWindow *ui;
    std::shared_ptr<Config> config;
    void initRulesWindow();
    void loadRule();

signals:
    void closeWindowSignal();

private slots:
    void on_btn_ok_clicked();
    void on_btn_rst_clicked();
};

#endif // RULESWINDOW_H
