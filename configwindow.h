#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

#include <QWidget>

#include "config.h"

namespace Ui {
class ConfigWindow;
}

class ConfigWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigWindow(QWidget *parent = nullptr, std::shared_ptr<Config> config = nullptr);
    ~ConfigWindow();
    void closeEvent(QCloseEvent *);

private:
    Ui::ConfigWindow *ui;
    std::shared_ptr<Config> config;
    void loadOriginValue();

signals:
    void closeWindowSignal();

private slots:
    void on_btn_rst_clicked();
    void on_btn_ok_clicked();
    void on_edit_addr_editingFinished();
};

#endif // CONFIGWINDOW_H
