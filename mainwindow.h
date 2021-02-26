#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QProcess>
#include <QAction>
#include <QFileDialog>
#include <QInputDialog>

#include <memory>
#include <thread>

#include "config.h"
#include "configwindow.h"

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
    void onconfig_log();
    void onconfig_core();
    void onconfig_local();
    void onconfig_server();

private:
    Ui::MainWindow *ui;
    std::shared_ptr<Config> config;
    std::unique_ptr<QProcess> core_process;
    std::unique_ptr<ConfigWindow> config_window;
};
#endif // MAINWINDOW_H
