#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QProcess>
#include <QAction>
#include <QFileDialog>
#include <QInputDialog>
#include <QListWidget>
#include <QUuid>
#include <QTimer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QScrollBar>
#include <QDesktopWidget>

#include <memory>
#include <thread>

#include "utils.h"
#include "config.h"
#include "configwindow.h"
#include "ruleslist.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void oncore_start();
    void oncore_stop();
    void oncore_error(QProcess::ProcessError);
    void onread_output();
    void onstat_update();

    void onconfig_log();
    void onconfig_core();
    void onconfig_local();
    void onconfig_server();
    void onconfig_api();
    void onconfig_rules();
    void onconfig_reuse();
    void onconfig_limit();
    void onconfig_padding();

    void onget_traffic_callback(QNetworkReply*);
    void onget_conn_callback(QNetworkReply*);

private:
    const QString api_token;
    Ui::MainWindow *ui;
    std::unique_ptr<QTimer> timer;
    std::unique_ptr<QNetworkAccessManager> netman_traffic;
    std::unique_ptr<QNetworkAccessManager> netman_conn;
    std::shared_ptr<Config> config;
    std::unique_ptr<QProcess> core_process;
    std::unique_ptr<ConfigWindow> config_window;
    std::unique_ptr<RulesList> rules_list;
};
#endif // MAINWINDOW_H
